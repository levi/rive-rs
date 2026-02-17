#include "rive_rs_abi.h"

#include "rive/animation/animation_state.hpp"
#include "rive/animation/any_state.hpp"
#include "rive/animation/entry_state.hpp"
#include "rive/animation/exit_state.hpp"
#include "rive/animation/linear_animation.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/nested_state_machine.hpp"
#include "rive/animation/state_machine.hpp"
#include "rive/animation/state_machine_bool.hpp"
#include "rive/animation/state_machine_input_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/animation/state_machine_number.hpp"
#include "rive/animation/state_machine_trigger.hpp"
#include "rive/artboard.hpp"
#include "rive/assets/audio_asset.hpp"
#include "rive/assets/file_asset.hpp"
#include "rive/assets/font_asset.hpp"
#include "rive/assets/image_asset.hpp"
#include "rive/bindable_artboard.hpp"
#include "rive/bones/bone.hpp"
#include "rive/bones/root_bone.hpp"
#include "rive/constraints/constraint.hpp"
#include "rive/custom_property.hpp"
#include "rive/custom_property_boolean.hpp"
#include "rive/custom_property_number.hpp"
#include "rive/custom_property_string.hpp"
#include "rive/event.hpp"
#include "rive/factory.hpp"
#include "rive/file.hpp"
#include "rive/file_asset_loader.hpp"
#include "rive/math/mat2d.hpp"
#include "rive/math/vec2d.hpp"
#include "rive/nested_artboard.hpp"
#include "rive/node.hpp"
#include "rive/open_url_event.hpp"
#include "rive/renderer.hpp"
#include "rive/shapes/cubic_vertex.hpp"
#include "rive/shapes/path.hpp"
#include "rive/simple_array.hpp"
#include "rive/text/text_value_run.hpp"
#include "rive/text_engine.hpp"
#include "rive/transform_component.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_artboard_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_boolean_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_color_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_enum_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_list_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_number_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_string_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_trigger_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_instance_value_runtime.hpp"
#include "rive/viewmodel/runtime/viewmodel_runtime.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <string>
#include <utility>
#include <vector>

struct rive_rs_factory
{
    std::atomic_uint32_t refs;
    rive::Factory* factory;
};

struct rive_rs_artboard
{
    std::atomic_uint32_t refs;
    std::unique_ptr<rive::ArtboardInstance> artboard;
};

namespace
{
class NoOpRenderImage final : public rive::RenderImage
{};

class NoOpRenderPaint final : public rive::RenderPaint
{
public:
    void color(unsigned int) override {}
    void style(rive::RenderPaintStyle) override {}
    void thickness(float) override {}
    void join(rive::StrokeJoin) override {}
    void cap(rive::StrokeCap) override {}
    void blendMode(rive::BlendMode) override {}
    void shader(rive::rcp<rive::RenderShader>) override {}
    void invalidateStroke() override {}
    void feather(float) override {}
};

class NoOpRenderPath final : public rive::RenderPath
{
public:
    void rewind() override {}
    void fillRule(rive::FillRule) override {}
    void addPath(rive::CommandPath*, const rive::Mat2D&) override {}
    void addRenderPath(rive::RenderPath*, const rive::Mat2D&) override {}
    void moveTo(float, float) override {}
    void lineTo(float, float) override {}
    void cubicTo(float, float, float, float, float, float) override {}
    void close() override {}
    void addRawPath(const rive::RawPath&) override {}
};

class NoOpRenderShader final : public rive::RenderShader
{};

class NoOpFactory final : public rive::Factory
{
public:
    rive::rcp<rive::RenderBuffer> makeRenderBuffer(rive::RenderBufferType,
                                                    rive::RenderBufferFlags,
                                                    size_t) override
    {
        return nullptr;
    }

    rive::rcp<rive::RenderShader> makeLinearGradient(float,
                                                     float,
                                                     float,
                                                     float,
                                                     const rive::ColorInt[],
                                                     const float[],
                                                     size_t) override
    {
        return rive::make_rcp<NoOpRenderShader>();
    }

    rive::rcp<rive::RenderShader> makeRadialGradient(float,
                                                     float,
                                                     float,
                                                     const rive::ColorInt[],
                                                     const float[],
                                                     size_t) override
    {
        return rive::make_rcp<NoOpRenderShader>();
    }

    rive::rcp<rive::RenderPath> makeRenderPath(rive::RawPath&, rive::FillRule) override
    {
        return rive::make_rcp<NoOpRenderPath>();
    }

    rive::rcp<rive::RenderPath> makeEmptyRenderPath() override
    {
        return rive::make_rcp<NoOpRenderPath>();
    }

    rive::rcp<rive::RenderPaint> makeRenderPaint() override
    {
        return rive::make_rcp<NoOpRenderPaint>();
    }

    rive::rcp<rive::RenderImage> decodeImage(rive::Span<const uint8_t>) override
    {
        return rive::make_rcp<NoOpRenderImage>();
    }
};

constexpr rive_rs_str_view kEmptyStrView{nullptr, 0};
thread_local std::string g_scratch_string;

inline void factory_ref_internal(rive_rs_factory* factory)
{
    if (factory != nullptr)
    {
        (void)factory->refs.fetch_add(1, std::memory_order_relaxed);
    }
}

inline void factory_unref_internal(rive_rs_factory* factory)
{
    if (factory == nullptr)
    {
        return;
    }
    if (factory->refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        delete factory->factory;
        delete factory;
    }
}

inline rive::Factory* as_factory(rive_rs_factory* factory)
{
    return factory == nullptr ? nullptr : factory->factory;
}

inline const rive::Factory* as_factory(const rive_rs_factory* factory)
{
    return factory == nullptr ? nullptr : factory->factory;
}

inline rive::ArtboardInstance* as_artboard(rive_rs_artboard* artboard)
{
    return artboard == nullptr ? nullptr : artboard->artboard.get();
}

inline rive::ArtboardInstance* as_artboard(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? nullptr : artboard->artboard.get();
}

inline rive::File* as_file(rive_rs_file* file)
{
    return reinterpret_cast<rive::File*>(file);
}

inline const rive::File* as_file(const rive_rs_file* file)
{
    return reinterpret_cast<const rive::File*>(file);
}

inline rive::ViewModelRuntime* as_view_model(rive_rs_view_model* view_model)
{
    return reinterpret_cast<rive::ViewModelRuntime*>(view_model);
}

inline rive::ViewModelRuntime* as_view_model(const rive_rs_view_model* view_model)
{
    return reinterpret_cast<rive::ViewModelRuntime*>(const_cast<rive_rs_view_model*>(view_model));
}

inline rive::ViewModelInstanceRuntime* as_view_model_instance(
    rive_rs_view_model_instance* instance)
{
    return reinterpret_cast<rive::ViewModelInstanceRuntime*>(instance);
}

inline const rive::ViewModelInstanceRuntime* as_view_model_instance(
    const rive_rs_view_model_instance* instance)
{
    return reinterpret_cast<const rive::ViewModelInstanceRuntime*>(instance);
}

inline rive::BindableArtboard* as_bindable_artboard(
    rive_rs_bindable_artboard* bindable_artboard)
{
    return reinterpret_cast<rive::BindableArtboard*>(bindable_artboard);
}

inline const rive::BindableArtboard* as_bindable_artboard(
    const rive_rs_bindable_artboard* bindable_artboard)
{
    return reinterpret_cast<const rive::BindableArtboard*>(bindable_artboard);
}

inline rive::TransformComponent* as_transform_component(
    rive_rs_transform_component* component)
{
    return reinterpret_cast<rive::TransformComponent*>(component);
}

inline const rive::TransformComponent* as_transform_component(
    const rive_rs_transform_component* component)
{
    return reinterpret_cast<const rive::TransformComponent*>(component);
}

inline rive::Node* as_node(rive_rs_node* node)
{
    return reinterpret_cast<rive::Node*>(node);
}

inline const rive::Node* as_node(const rive_rs_node* node)
{
    return reinterpret_cast<const rive::Node*>(node);
}

inline rive::Bone* as_bone(rive_rs_bone* bone)
{
    return reinterpret_cast<rive::Bone*>(bone);
}

inline const rive::Bone* as_bone(const rive_rs_bone* bone)
{
    return reinterpret_cast<const rive::Bone*>(bone);
}

inline rive::RootBone* as_root_bone(rive_rs_root_bone* root_bone)
{
    return reinterpret_cast<rive::RootBone*>(root_bone);
}

inline const rive::RootBone* as_root_bone(const rive_rs_root_bone* root_bone)
{
    return reinterpret_cast<const rive::RootBone*>(root_bone);
}

inline rive::TextValueRun* as_text_value_run(rive_rs_text_value_run* text_value_run)
{
    return reinterpret_cast<rive::TextValueRun*>(text_value_run);
}

inline const rive::TextValueRun* as_text_value_run(
    const rive_rs_text_value_run* text_value_run)
{
    return reinterpret_cast<const rive::TextValueRun*>(text_value_run);
}

inline rive::LinearAnimation* as_linear_animation(rive_rs_linear_animation* animation)
{
    return reinterpret_cast<rive::LinearAnimation*>(animation);
}

inline const rive::LinearAnimation* as_linear_animation(
    const rive_rs_linear_animation* animation)
{
    return reinterpret_cast<const rive::LinearAnimation*>(animation);
}

inline rive::LinearAnimationInstance* as_linear_animation_instance(
    rive_rs_linear_animation_instance* instance)
{
    return reinterpret_cast<rive::LinearAnimationInstance*>(instance);
}

inline const rive::LinearAnimationInstance* as_linear_animation_instance(
    const rive_rs_linear_animation_instance* instance)
{
    return reinterpret_cast<const rive::LinearAnimationInstance*>(instance);
}

inline rive::StateMachine* as_state_machine(rive_rs_state_machine* state_machine)
{
    return reinterpret_cast<rive::StateMachine*>(state_machine);
}

inline const rive::StateMachine* as_state_machine(const rive_rs_state_machine* state_machine)
{
    return reinterpret_cast<const rive::StateMachine*>(state_machine);
}

inline rive::StateMachineInstance* as_state_machine_instance(
    rive_rs_state_machine_instance* instance)
{
    return reinterpret_cast<rive::StateMachineInstance*>(instance);
}

inline const rive::StateMachineInstance* as_state_machine_instance(
    const rive_rs_state_machine_instance* instance)
{
    return reinterpret_cast<const rive::StateMachineInstance*>(instance);
}

inline rive::SMIInput* as_smi_input(rive_rs_smi_input* input)
{
    return reinterpret_cast<rive::SMIInput*>(input);
}

inline const rive::SMIInput* as_smi_input(const rive_rs_smi_input* input)
{
    return reinterpret_cast<const rive::SMIInput*>(input);
}

inline rive::SMIBool* as_smi_bool(rive_rs_smi_bool* input)
{
    return reinterpret_cast<rive::SMIBool*>(input);
}

inline const rive::SMIBool* as_smi_bool(const rive_rs_smi_bool* input)
{
    return reinterpret_cast<const rive::SMIBool*>(input);
}

inline rive::SMINumber* as_smi_number(rive_rs_smi_number* input)
{
    return reinterpret_cast<rive::SMINumber*>(input);
}

inline const rive::SMINumber* as_smi_number(const rive_rs_smi_number* input)
{
    return reinterpret_cast<const rive::SMINumber*>(input);
}

inline rive::SMITrigger* as_smi_trigger(rive_rs_smi_trigger* input)
{
    return reinterpret_cast<rive::SMITrigger*>(input);
}

inline const rive::SMITrigger* as_smi_trigger(const rive_rs_smi_trigger* input)
{
    return reinterpret_cast<const rive::SMITrigger*>(input);
}

inline rive::FileAsset* as_file_asset(rive_rs_file_asset* asset)
{
    return reinterpret_cast<rive::FileAsset*>(asset);
}

inline const rive::FileAsset* as_file_asset(const rive_rs_file_asset* asset)
{
    return reinterpret_cast<const rive::FileAsset*>(asset);
}

inline rive::AudioSource* as_audio_source(rive_rs_audio_source* audio)
{
    return reinterpret_cast<rive::AudioSource*>(audio);
}

inline const rive::AudioSource* as_audio_source(const rive_rs_audio_source* audio)
{
    return reinterpret_cast<const rive::AudioSource*>(audio);
}

inline rive::Font* as_font(rive_rs_font* font)
{
    return reinterpret_cast<rive::Font*>(font);
}

inline const rive::Font* as_font(const rive_rs_font* font)
{
    return reinterpret_cast<const rive::Font*>(font);
}

#ifdef ENABLE_QUERY_FLAT_VERTICES
inline rive::FlattenedPath* as_flattened_path(rive_rs_flattened_path* path)
{
    return reinterpret_cast<rive::FlattenedPath*>(path);
}

inline const rive::FlattenedPath* as_flattened_path(const rive_rs_flattened_path* path)
{
    return reinterpret_cast<const rive::FlattenedPath*>(path);
}
#endif

inline rive_rs_str_view to_str_view(const std::string& value)
{
    return rive_rs_str_view{value.c_str(), value.size()};
}

inline rive_rs_str_view to_str_view(const char* literal)
{
    if (literal == nullptr)
    {
        return kEmptyStrView;
    }
    return rive_rs_str_view{literal, std::strlen(literal)};
}

inline rive_rs_str_view to_scratch_view(std::string value)
{
    g_scratch_string = std::move(value);
    return to_str_view(g_scratch_string);
}

inline std::string from_str_view(rive_rs_str_view view)
{
    if (view.ptr == nullptr || view.len == 0)
    {
        return std::string();
    }
    return std::string(view.ptr, view.len);
}

inline bool invalid_bytes(rive_rs_bytes_view bytes)
{
    return bytes.ptr == nullptr && bytes.len > 0;
}

inline rive::Span<const uint8_t> to_span(rive_rs_bytes_view bytes)
{
    return rive::Span<const uint8_t>(bytes.ptr, bytes.len);
}

inline rive::SimpleArray<uint8_t> to_simple_array(rive_rs_bytes_view bytes)
{
    rive::SimpleArray<uint8_t> data(bytes.len);
    if (bytes.len > 0)
    {
        std::memcpy(data.data(), bytes.ptr, bytes.len);
    }
    return data;
}

inline rive::Mat2D to_mat2d(const rive_rs_mat2d& matrix)
{
    return rive::Mat2D(matrix.xx,
                       matrix.xy,
                       matrix.yx,
                       matrix.yy,
                       matrix.tx,
                       matrix.ty);
}

inline void copy_mat2d(const rive::Mat2D& matrix, rive_rs_mat2d* out_matrix)
{
    out_matrix->xx = matrix.xx();
    out_matrix->xy = matrix.xy();
    out_matrix->yx = matrix.yx();
    out_matrix->yy = matrix.yy();
    out_matrix->tx = matrix.tx();
    out_matrix->ty = matrix.ty();
}

inline rive_rs_aabb to_abi_aabb(const rive::AABB& aabb)
{
    return rive_rs_aabb{aabb.minX, aabb.minY, aabb.maxX, aabb.maxY};
}

inline rive::AABB to_runtime_aabb(const rive_rs_aabb& aabb)
{
    return rive::AABB(aabb.min_x, aabb.min_y, aabb.max_x, aabb.max_y);
}

inline rive::Vec2D to_runtime_vec2(rive_rs_vec2 vec)
{
    return rive::Vec2D(vec.x, vec.y);
}

inline rive_rs_data_type to_abi_data_type(rive::DataType data_type)
{
    switch (data_type)
    {
        case rive::DataType::string:
            return RIVE_RS_DATA_TYPE_STRING;
        case rive::DataType::number:
            return RIVE_RS_DATA_TYPE_NUMBER;
        case rive::DataType::boolean:
            return RIVE_RS_DATA_TYPE_BOOLEAN;
        case rive::DataType::color:
            return RIVE_RS_DATA_TYPE_COLOR;
        case rive::DataType::list:
            return RIVE_RS_DATA_TYPE_LIST;
        case rive::DataType::enumType:
            return RIVE_RS_DATA_TYPE_ENUM;
        case rive::DataType::trigger:
            return RIVE_RS_DATA_TYPE_TRIGGER;
        case rive::DataType::viewModel:
            return RIVE_RS_DATA_TYPE_VIEW_MODEL;
        case rive::DataType::integer:
            return RIVE_RS_DATA_TYPE_INTEGER;
        case rive::DataType::symbolListIndex:
            return RIVE_RS_DATA_TYPE_LIST_INDEX;
        case rive::DataType::assetImage:
            return RIVE_RS_DATA_TYPE_IMAGE;
        case rive::DataType::artboard:
            return RIVE_RS_DATA_TYPE_ARTBOARD;
        case rive::DataType::none:
        case rive::DataType::input:
        case rive::DataType::any:
        default:
            return RIVE_RS_DATA_TYPE_NONE;
    }
}

inline bool to_runtime_fit(rive_rs_fit fit, rive::Fit* out_fit)
{
    if (out_fit == nullptr)
    {
        return false;
    }
    switch (fit)
    {
        case RIVE_RS_FIT_FILL:
            *out_fit = rive::Fit::fill;
            return true;
        case RIVE_RS_FIT_CONTAIN:
            *out_fit = rive::Fit::contain;
            return true;
        case RIVE_RS_FIT_COVER:
            *out_fit = rive::Fit::cover;
            return true;
        case RIVE_RS_FIT_FIT_WIDTH:
            *out_fit = rive::Fit::fitWidth;
            return true;
        case RIVE_RS_FIT_FIT_HEIGHT:
            *out_fit = rive::Fit::fitHeight;
            return true;
        case RIVE_RS_FIT_NONE:
            *out_fit = rive::Fit::none;
            return true;
        case RIVE_RS_FIT_SCALE_DOWN:
            *out_fit = rive::Fit::scaleDown;
            return true;
        case RIVE_RS_FIT_LAYOUT:
            *out_fit = rive::Fit::layout;
            return true;
        default:
            return false;
    }
}

inline bool to_runtime_alignment(rive_rs_alignment alignment,
                                 rive::Alignment* out_alignment)
{
    if (out_alignment == nullptr)
    {
        return false;
    }
    switch (alignment)
    {
        case RIVE_RS_ALIGNMENT_TOP_LEFT:
            *out_alignment = rive::Alignment::topLeft;
            return true;
        case RIVE_RS_ALIGNMENT_TOP_CENTER:
            *out_alignment = rive::Alignment::topCenter;
            return true;
        case RIVE_RS_ALIGNMENT_TOP_RIGHT:
            *out_alignment = rive::Alignment::topRight;
            return true;
        case RIVE_RS_ALIGNMENT_CENTER_LEFT:
            *out_alignment = rive::Alignment::centerLeft;
            return true;
        case RIVE_RS_ALIGNMENT_CENTER:
            *out_alignment = rive::Alignment::center;
            return true;
        case RIVE_RS_ALIGNMENT_CENTER_RIGHT:
            *out_alignment = rive::Alignment::centerRight;
            return true;
        case RIVE_RS_ALIGNMENT_BOTTOM_LEFT:
            *out_alignment = rive::Alignment::bottomLeft;
            return true;
        case RIVE_RS_ALIGNMENT_BOTTOM_CENTER:
            *out_alignment = rive::Alignment::bottomCenter;
            return true;
        case RIVE_RS_ALIGNMENT_BOTTOM_RIGHT:
            *out_alignment = rive::Alignment::bottomRight;
            return true;
        default:
            return false;
    }
}

inline rive_rs_status import_result_to_status(rive::ImportResult result)
{
    switch (result)
    {
        case rive::ImportResult::success:
            return RIVE_RS_STATUS_OK;
        case rive::ImportResult::unsupportedVersion:
        case rive::ImportResult::malformed:
        default:
            return RIVE_RS_STATUS_DECODE_ERROR;
    }
}

inline rive_rs_artboard* new_artboard_handle(
    std::unique_ptr<rive::ArtboardInstance> artboard)
{
    if (artboard == nullptr)
    {
        return nullptr;
    }
    auto* handle = new (std::nothrow) rive_rs_artboard();
    if (handle == nullptr)
    {
        return nullptr;
    }
    handle->refs.store(1, std::memory_order_relaxed);
    handle->artboard = std::move(artboard);
    return handle;
}

inline void artboard_ref_internal(rive_rs_artboard* artboard)
{
    if (artboard != nullptr)
    {
        (void)artboard->refs.fetch_add(1, std::memory_order_relaxed);
    }
}

inline void artboard_unref_internal(rive_rs_artboard* artboard)
{
    if (artboard == nullptr)
    {
        return;
    }
    if (artboard->refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        delete artboard;
    }
}

inline size_t custom_property_count(const rive::Event* event)
{
    if (event == nullptr)
    {
        return 0;
    }

    size_t count = 0;
    for (auto* child : event->children())
    {
        if (child != nullptr && child->is<rive::CustomProperty>() &&
            !child->name().empty())
        {
            count++;
        }
    }
    return count;
}

inline rive::CustomProperty* custom_property_at(rive::Event* event, size_t index)
{
    if (event == nullptr)
    {
        return nullptr;
    }

    size_t count = 0;
    for (auto* child : event->children())
    {
        if (child != nullptr && child->is<rive::CustomProperty>() &&
            !child->name().empty())
        {
            if (count == index)
            {
                return child->as<rive::CustomProperty>();
            }
            count++;
        }
    }
    return nullptr;
}

inline const char* open_url_target(uint32_t target)
{
    switch (target)
    {
        case 0:
            return "_blank";
        case 1:
            return "_parent";
        case 2:
            return "_self";
        case 3:
            return "_top";
        default:
            return nullptr;
    }
}

inline void fill_event_info(const rive::Event* event, rive_rs_event_info* out_event)
{
    out_event->name = to_str_view(event->name());
    out_event->type = event->coreType();
    out_event->has_url = false;
    out_event->url = kEmptyStrView;
    out_event->has_target = false;
    out_event->target = kEmptyStrView;

    if (event->is<rive::OpenUrlEvent>())
    {
        const auto* open_url_event = event->as<rive::OpenUrlEvent>();
        if (!open_url_event->url().empty())
        {
            out_event->has_url = true;
            out_event->url = to_str_view(open_url_event->url());
        }

        if (const char* target = open_url_target(open_url_event->targetValue());
            target != nullptr)
        {
            out_event->has_target = true;
            out_event->target = to_str_view(target);
        }
    }

    out_event->property_count = custom_property_count(event);
}

inline rive_rs_status fill_event_property_info(const rive::CustomProperty* property,
                                               rive_rs_event_property_info* out_property)
{
    if (property == nullptr || out_property == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    out_property->name = to_str_view(property->name());
    out_property->bool_value = false;
    out_property->number_value = 0.0f;
    out_property->string_value = kEmptyStrView;

    switch (property->coreType())
    {
        case rive::CustomPropertyBoolean::typeKey:
            out_property->value_type = RIVE_RS_EVENT_PROPERTY_BOOL;
            out_property->bool_value =
                property->as<rive::CustomPropertyBoolean>()->propertyValue();
            return RIVE_RS_STATUS_OK;
        case rive::CustomPropertyNumber::typeKey:
            out_property->value_type = RIVE_RS_EVENT_PROPERTY_NUMBER;
            out_property->number_value =
                property->as<rive::CustomPropertyNumber>()->propertyValue();
            return RIVE_RS_STATUS_OK;
        case rive::CustomPropertyString::typeKey:
            out_property->value_type = RIVE_RS_EVENT_PROPERTY_STRING;
            out_property->string_value =
                to_str_view(property->as<rive::CustomPropertyString>()->propertyValue());
            return RIVE_RS_STATUS_OK;
        default:
            return RIVE_RS_STATUS_UNSUPPORTED;
    }
}

inline bool has_any_listener(rive::StateMachineInstance* instance)
{
    if (instance == nullptr)
    {
        return false;
    }
    if (instance->hasListeners())
    {
        return true;
    }
    for (auto* nested_artboard : instance->artboard()->nestedArtboards())
    {
        for (auto* nested_animation : nested_artboard->nestedAnimations())
        {
            if (nested_animation->is<rive::NestedStateMachine>())
            {
                auto* nested_state_machine = nested_animation->as<rive::NestedStateMachine>();
                if (has_any_listener(nested_state_machine->stateMachineInstance()))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

class AbiFileAssetLoader final : public rive::FileAssetLoader
{
public:
    AbiFileAssetLoader(const rive_rs_file_asset_loader_callbacks& callbacks,
                       rive_rs_factory* factory_handle) :
        m_callbacks(callbacks), m_factory(factory_handle)
    {
        factory_ref_internal(m_factory);
    }

    ~AbiFileAssetLoader() override { factory_unref_internal(m_factory); }

    bool loadContents(rive::FileAsset& asset,
                      rive::Span<const uint8_t> in_band_bytes,
                      rive::Factory*) override
    {
        if (m_callbacks.load_contents == nullptr)
        {
            return false;
        }

        rive_rs_bytes_view bytes{in_band_bytes.data(), in_band_bytes.size()};
        return m_callbacks.load_contents(m_callbacks.user_data,
                                         reinterpret_cast<rive_rs_file_asset*>(&asset),
                                         bytes,
                                         m_factory);
    }

private:
    rive_rs_file_asset_loader_callbacks m_callbacks{};
    rive_rs_factory* m_factory = nullptr;
};
} // namespace

extern "C"
{
uint32_t rive_rs_abi_version(void) { return 1; }

rive_rs_factory* rive_rs_factory_default(void)
{
    auto* handle = new (std::nothrow) rive_rs_factory();
    if (handle == nullptr)
    {
        return nullptr;
    }

    auto* factory = new (std::nothrow) NoOpFactory();
    if (factory == nullptr)
    {
        delete handle;
        return nullptr;
    }

    handle->refs.store(1, std::memory_order_relaxed);
    handle->factory = factory;
    return handle;
}

void rive_rs_factory_ref(rive_rs_factory* factory) { factory_ref_internal(factory); }

void rive_rs_factory_unref(rive_rs_factory* factory)
{
    factory_unref_internal(factory);
}

rive_rs_status rive_rs_load_file(rive_rs_factory* factory,
                                 rive_rs_bytes_view bytes,
                                 rive_rs_file** out_file)
{
    if (out_file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_file = nullptr;

    if (factory == nullptr || as_factory(factory) == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (invalid_bytes(bytes))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    rive::ImportResult import_result = rive::ImportResult::malformed;
    auto file = rive::File::import(to_span(bytes), as_factory(factory), &import_result, nullptr);
    if (file == nullptr)
    {
        return import_result_to_status(import_result);
    }

    *out_file = reinterpret_cast<rive_rs_file*>(file.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_load_file_with_asset_loader(
    rive_rs_factory* factory,
    rive_rs_bytes_view bytes,
    const rive_rs_file_asset_loader_callbacks* callbacks,
    rive_rs_file** out_file)
{
    if (out_file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_file = nullptr;

    if (factory == nullptr || as_factory(factory) == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (invalid_bytes(bytes))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    rive::ImportResult import_result = rive::ImportResult::malformed;

    if (callbacks == nullptr || callbacks->load_contents == nullptr)
    {
        auto file =
            rive::File::import(to_span(bytes), as_factory(factory), &import_result, nullptr);
        if (file == nullptr)
        {
            return import_result_to_status(import_result);
        }
        *out_file = reinterpret_cast<rive_rs_file*>(file.release());
        return RIVE_RS_STATUS_OK;
    }

    auto loader = rive::make_rcp<AbiFileAssetLoader>(*callbacks, factory);
    auto file =
        rive::File::import(to_span(bytes), as_factory(factory), &import_result, loader.get());
    if (file == nullptr)
    {
        return import_result_to_status(import_result);
    }

    *out_file = reinterpret_cast<rive_rs_file*>(file.release());
    return RIVE_RS_STATUS_OK;
}

void rive_rs_file_ref(rive_rs_file* file)
{
    if (file != nullptr)
    {
        as_file(file)->ref();
    }
}

void rive_rs_file_unref(rive_rs_file* file)
{
    if (file != nullptr)
    {
        as_file(file)->unref();
    }
}

size_t rive_rs_file_artboard_count(const rive_rs_file* file)
{
    return file == nullptr ? 0 : as_file(file)->artboardCount();
}

rive_rs_status rive_rs_file_artboard_default(const rive_rs_file* file,
                                             rive_rs_artboard** out_artboard)
{
    if (out_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_artboard = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto artboard = as_file(file)->artboardDefault();
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto* handle = new_artboard_handle(std::move(artboard));
    if (handle == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_artboard = handle;
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_artboard_by_index(const rive_rs_file* file,
                                              size_t index,
                                              rive_rs_artboard** out_artboard)
{
    if (out_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_artboard = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (index >= as_file(file)->artboardCount())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto artboard = as_file(file)->artboardAt(index);
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto* handle = new_artboard_handle(std::move(artboard));
    if (handle == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_artboard = handle;
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_artboard_by_name(const rive_rs_file* file,
                                             rive_rs_str_view name,
                                             rive_rs_artboard** out_artboard)
{
    if (out_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_artboard = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto artboard = as_file(file)->artboardNamed(from_str_view(name));
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto* handle = new_artboard_handle(std::move(artboard));
    if (handle == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_artboard = handle;
    return RIVE_RS_STATUS_OK;
}

size_t rive_rs_file_view_model_count(const rive_rs_file* file)
{
    return file == nullptr ? 0 : as_file(file)->viewModelCount();
}

rive_rs_status rive_rs_file_view_model_by_index(const rive_rs_file* file,
                                                size_t index,
                                                rive_rs_view_model** out_view_model)
{
    if (out_view_model == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_view_model = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (index >= as_file(file)->viewModelCount())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* view_model = as_file(file)->viewModelByIndex(index);
    if (view_model == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    view_model->ref();
    *out_view_model = reinterpret_cast<rive_rs_view_model*>(view_model);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_view_model_by_name(const rive_rs_file* file,
                                               rive_rs_str_view name,
                                               rive_rs_view_model** out_view_model)
{
    if (out_view_model == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_view_model = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* view_model = as_file(file)->viewModelByName(from_str_view(name));
    if (view_model == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    view_model->ref();
    *out_view_model = reinterpret_cast<rive_rs_view_model*>(view_model);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_default_artboard_view_model(
    const rive_rs_file* file,
    rive_rs_artboard* artboard,
    rive_rs_view_model** out_view_model)
{
    if (out_view_model == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_view_model = nullptr;

    if (file == nullptr || artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* view_model =
        as_file(file)->defaultArtboardViewModel(as_artboard(artboard));
    if (view_model == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    view_model->ref();
    *out_view_model = reinterpret_cast<rive_rs_view_model*>(view_model);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_bindable_artboard_by_name(
    const rive_rs_file* file,
    rive_rs_str_view name,
    rive_rs_bindable_artboard** out_bindable_artboard)
{
    if (out_bindable_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_bindable_artboard = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto bindable = as_file(file)->bindableArtboardNamed(from_str_view(name));
    if (bindable == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_bindable_artboard =
        reinterpret_cast<rive_rs_bindable_artboard*>(bindable.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_bindable_artboard_default(
    const rive_rs_file* file,
    rive_rs_bindable_artboard** out_bindable_artboard)
{
    if (out_bindable_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_bindable_artboard = nullptr;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto bindable = as_file(file)->bindableArtboardDefault();
    if (bindable == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_bindable_artboard =
        reinterpret_cast<rive_rs_bindable_artboard*>(bindable.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_file_bindable_artboard_from_artboard(
    const rive_rs_file* file,
    rive_rs_artboard* artboard,
    rive_rs_bindable_artboard** out_bindable_artboard)
{
    if (out_bindable_artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_bindable_artboard = nullptr;

    if (file == nullptr || artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto bindable =
        as_file(file)->internalBindableArtboardFromArtboard(as_artboard(artboard));
    if (bindable == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_bindable_artboard =
        reinterpret_cast<rive_rs_bindable_artboard*>(bindable.release());
    return RIVE_RS_STATUS_OK;
}

bool rive_rs_file_has_audio(const rive_rs_file* file)
{
    return file == nullptr ? false : as_file(file)->hasAudio();
}

size_t rive_rs_file_enum_count(const rive_rs_file* file)
{
    if (file == nullptr)
    {
        return 0;
    }
    return as_file(file)->enums().size();
}

rive_rs_status rive_rs_file_enum_name_at(const rive_rs_file* file,
                                         size_t enum_index,
                                         rive_rs_str_view* out_name)
{
    if (out_name == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_name = kEmptyStrView;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& enums = as_file(file)->enums();
    if (enum_index >= enums.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* data_enum = enums[enum_index];
    if (data_enum == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_name = to_str_view(data_enum->enumName());
    return RIVE_RS_STATUS_OK;
}

size_t rive_rs_file_enum_value_count(const rive_rs_file* file, size_t enum_index)
{
    if (file == nullptr)
    {
        return 0;
    }

    const auto& enums = as_file(file)->enums();
    if (enum_index >= enums.size())
    {
        return 0;
    }

    auto* data_enum = enums[enum_index];
    if (data_enum == nullptr)
    {
        return 0;
    }

    return data_enum->values().size();
}

rive_rs_status rive_rs_file_enum_value_name_at(const rive_rs_file* file,
                                               size_t enum_index,
                                               size_t value_index,
                                               rive_rs_str_view* out_name)
{
    if (out_name == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_name = kEmptyStrView;

    if (file == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& enums = as_file(file)->enums();
    if (enum_index >= enums.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* data_enum = enums[enum_index];
    if (data_enum == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto& values = data_enum->values();
    if (value_index >= values.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* value = values[value_index];
    if (value == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_name = to_str_view(value->key());
    return RIVE_RS_STATUS_OK;
}

void rive_rs_artboard_ref(rive_rs_artboard* artboard)
{
    artboard_ref_internal(artboard);
}

void rive_rs_artboard_unref(rive_rs_artboard* artboard)
{
    artboard_unref_internal(artboard);
}

rive_rs_status rive_rs_artboard_advance(rive_rs_artboard* artboard,
                                        float seconds,
                                        bool* out_changed)
{
    if (artboard == nullptr || out_changed == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_changed = as_artboard(artboard)->advance(seconds);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_draw(rive_rs_artboard* artboard,
                                     rive_rs_renderer* renderer)
{
    if (artboard == nullptr || renderer == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    as_artboard(artboard)->draw(reinterpret_cast<rive::Renderer*>(renderer));
    return RIVE_RS_STATUS_OK;
}

bool rive_rs_artboard_did_change(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? false : as_artboard(artboard)->didChange();
}

rive_rs_str_view rive_rs_artboard_name(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? kEmptyStrView : to_str_view(as_artboard(artboard)->name());
}

rive_rs_aabb rive_rs_artboard_bounds(const rive_rs_artboard* artboard)
{
    if (artboard == nullptr)
    {
        return rive_rs_aabb{0.0f, 0.0f, 0.0f, 0.0f};
    }
    return to_abi_aabb(as_artboard(artboard)->bounds());
}

float rive_rs_artboard_width(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0.0f : as_artboard(artboard)->width();
}

float rive_rs_artboard_height(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0.0f : as_artboard(artboard)->height();
}

void rive_rs_artboard_set_width(rive_rs_artboard* artboard, float width)
{
    if (artboard != nullptr)
    {
        as_artboard(artboard)->width(width);
    }
}

void rive_rs_artboard_set_height(rive_rs_artboard* artboard, float height)
{
    if (artboard != nullptr)
    {
        as_artboard(artboard)->height(height);
    }
}

bool rive_rs_artboard_frame_origin(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? false : as_artboard(artboard)->frameOrigin();
}

void rive_rs_artboard_set_frame_origin(rive_rs_artboard* artboard, bool frame_origin)
{
    if (artboard != nullptr)
    {
        as_artboard(artboard)->frameOrigin(frame_origin);
    }
}

bool rive_rs_artboard_has_audio(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? false : as_artboard(artboard)->hasAudio();
}

float rive_rs_artboard_volume(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0.0f : as_artboard(artboard)->volume();
}

void rive_rs_artboard_set_volume(rive_rs_artboard* artboard, float volume)
{
    if (artboard != nullptr)
    {
        as_artboard(artboard)->volume(volume);
    }
}

rive_rs_status rive_rs_artboard_reset_size(rive_rs_artboard* artboard)
{
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    as_artboard(artboard)->resetSize();
    return RIVE_RS_STATUS_OK;
}

size_t rive_rs_artboard_animation_count(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0 : as_artboard(artboard)->animationCount();
}

size_t rive_rs_artboard_state_machine_count(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0 : as_artboard(artboard)->stateMachineCount();
}

size_t rive_rs_artboard_event_count(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0 : as_artboard(artboard)->count<rive::Event>();
}

rive_rs_status rive_rs_artboard_event_at(const rive_rs_artboard* artboard,
                                         size_t index,
                                         rive_rs_event_info* out_event)
{
    if (artboard == nullptr || out_event == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* event = as_artboard(artboard)->objectAt<rive::Event>(index);
    if (event == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    fill_event_info(event, out_event);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_event_property_at(
    const rive_rs_artboard* artboard,
    size_t event_index,
    size_t property_index,
    rive_rs_event_property_info* out_property)
{
    if (artboard == nullptr || out_property == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* event = as_artboard(artboard)->objectAt<rive::Event>(event_index);
    if (event == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* property = custom_property_at(event, property_index);
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    return fill_event_property_info(property, out_property);
}

rive_rs_status rive_rs_artboard_animation_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_linear_animation** out_animation)
{
    if (artboard == nullptr || out_animation == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_animation = nullptr;
    auto* animation = as_artboard(artboard)->animation(index);
    if (animation == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_animation = reinterpret_cast<rive_rs_linear_animation*>(animation);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_animation_by_name(rive_rs_artboard* artboard,
                                                  rive_rs_str_view name,
                                                  rive_rs_linear_animation** out_animation)
{
    if (artboard == nullptr || out_animation == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_animation = nullptr;
    auto* animation = as_artboard(artboard)->animation(from_str_view(name));
    if (animation == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_animation = reinterpret_cast<rive_rs_linear_animation*>(animation);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_state_machine_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_state_machine** out_state_machine)
{
    if (artboard == nullptr || out_state_machine == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_state_machine = nullptr;
    auto* state_machine = as_artboard(artboard)->stateMachine(index);
    if (state_machine == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_state_machine = reinterpret_cast<rive_rs_state_machine*>(state_machine);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_state_machine_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_state_machine** out_state_machine)
{
    if (artboard == nullptr || out_state_machine == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_state_machine = nullptr;
    auto* state_machine = as_artboard(artboard)->stateMachine(from_str_view(name));
    if (state_machine == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_state_machine = reinterpret_cast<rive_rs_state_machine*>(state_machine);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_input_by_path(rive_rs_artboard* artboard,
                                              rive_rs_str_view name,
                                              rive_rs_str_view path,
                                              rive_rs_smi_input** out_input)
{
    if (artboard == nullptr || out_input == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_input = nullptr;
    auto* input =
        as_artboard(artboard)->input(from_str_view(name), from_str_view(path));
    if (input == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_input = reinterpret_cast<rive_rs_smi_input*>(input);
    return RIVE_RS_STATUS_OK;
}

size_t rive_rs_artboard_text_value_run_count(const rive_rs_artboard* artboard)
{
    return artboard == nullptr ? 0 : as_artboard(artboard)->count<rive::TextValueRun>();
}

rive_rs_status rive_rs_artboard_text_value_run_name_at(const rive_rs_artboard* artboard,
                                                       size_t index,
                                                       rive_rs_str_view* out_name)
{
    if (artboard == nullptr || out_name == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_name = kEmptyStrView;
    auto* text_run = as_artboard(artboard)->objectAt<rive::TextValueRun>(index);
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_name = to_str_view(text_run->name());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_text_value_run_text_at(const rive_rs_artboard* artboard,
                                                       size_t index,
                                                       rive_rs_str_view* out_text)
{
    if (artboard == nullptr || out_text == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_text = kEmptyStrView;
    auto* text_run = as_artboard(artboard)->objectAt<rive::TextValueRun>(index);
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_text = to_str_view(text_run->text());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_set_text_value_run_text_at(rive_rs_artboard* artboard,
                                                           size_t index,
                                                           rive_rs_str_view text)
{
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* text_run = as_artboard(artboard)->objectAt<rive::TextValueRun>(index);
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    text_run->text(from_str_view(text));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_text_by_path_get(const rive_rs_artboard* artboard,
                                                 rive_rs_str_view name,
                                                 rive_rs_str_view path,
                                                 rive_rs_str_view* out_text)
{
    if (artboard == nullptr || out_text == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_text = kEmptyStrView;
    auto* text_run =
        as_artboard(artboard)->getTextRun(from_str_view(name), from_str_view(path));
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_text = to_str_view(text_run->text());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_text_by_path_set(rive_rs_artboard* artboard,
                                                 rive_rs_str_view name,
                                                 rive_rs_str_view path,
                                                 rive_rs_str_view text)
{
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* text_run =
        as_artboard(artboard)->getTextRun(from_str_view(name), from_str_view(path));
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    text_run->text(from_str_view(text));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_transform_component_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_transform_component** out_component)
{
    if (artboard == nullptr || out_component == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_component = nullptr;
    auto* component =
        as_artboard(artboard)->find<rive::TransformComponent>(from_str_view(name));
    if (component == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_component = reinterpret_cast<rive_rs_transform_component*>(component);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_node_by_name(rive_rs_artboard* artboard,
                                             rive_rs_str_view name,
                                             rive_rs_node** out_node)
{
    if (artboard == nullptr || out_node == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_node = nullptr;
    auto* node = as_artboard(artboard)->find<rive::Node>(from_str_view(name));
    if (node == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_node = reinterpret_cast<rive_rs_node*>(node);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_bone_by_name(rive_rs_artboard* artboard,
                                             rive_rs_str_view name,
                                             rive_rs_bone** out_bone)
{
    if (artboard == nullptr || out_bone == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_bone = nullptr;
    auto* bone = as_artboard(artboard)->find<rive::Bone>(from_str_view(name));
    if (bone == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_bone = reinterpret_cast<rive_rs_bone*>(bone);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_root_bone_by_name(rive_rs_artboard* artboard,
                                                  rive_rs_str_view name,
                                                  rive_rs_root_bone** out_root_bone)
{
    if (artboard == nullptr || out_root_bone == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_root_bone = nullptr;
    auto* root_bone =
        as_artboard(artboard)->find<rive::RootBone>(from_str_view(name));
    if (root_bone == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_root_bone = reinterpret_cast<rive_rs_root_bone*>(root_bone);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_text_value_run_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_text_value_run** out_text_value_run)
{
    if (artboard == nullptr || out_text_value_run == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_text_value_run = nullptr;
    auto* text_run =
        as_artboard(artboard)->find<rive::TextValueRun>(from_str_view(name));
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_text_value_run = reinterpret_cast<rive_rs_text_value_run*>(text_run);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_text_value_run_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_text_value_run** out_text_value_run)
{
    if (artboard == nullptr || out_text_value_run == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_text_value_run = nullptr;
    auto* text_run = as_artboard(artboard)->objectAt<rive::TextValueRun>(index);
    if (text_run == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_text_value_run = reinterpret_cast<rive_rs_text_value_run*>(text_run);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_artboard_flatten_path(rive_rs_artboard* artboard,
                                             size_t index,
                                             bool transform_to_parent,
                                             rive_rs_flattened_path** out_path)
{
    if (out_path == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    *out_path = nullptr;

    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

#ifdef ENABLE_QUERY_FLAT_VERTICES
    const auto& objects = as_artboard(artboard)->objects();
    if (index >= objects.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto* object = objects[index];
    if (object == nullptr || !object->is<rive::Path>())
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto* path = object->as<rive::Path>();
    auto* flattened = path->makeFlat(transform_to_parent);
    if (flattened == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_path = reinterpret_cast<rive_rs_flattened_path*>(flattened);
    return RIVE_RS_STATUS_OK;
#else
    (void)index;
    (void)transform_to_parent;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_artboard_bind_view_model_instance(
    rive_rs_artboard* artboard,
    rive_rs_view_model_instance* instance)
{
    if (artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto view_model_instance = instance == nullptr
                                   ? rive::rcp<rive::ViewModelInstance>(nullptr)
                                   : as_view_model_instance(instance)->instance();
    as_artboard(artboard)->bindViewModelInstance(view_model_instance);
    return RIVE_RS_STATUS_OK;
}

void rive_rs_bindable_artboard_ref(rive_rs_bindable_artboard* bindable_artboard)
{
    if (bindable_artboard != nullptr)
    {
        as_bindable_artboard(bindable_artboard)->ref();
    }
}

void rive_rs_bindable_artboard_unref(rive_rs_bindable_artboard* bindable_artboard)
{
    if (bindable_artboard != nullptr)
    {
        as_bindable_artboard(bindable_artboard)->unref();
    }
}

float rive_rs_transform_component_scale_x(const rive_rs_transform_component* component)
{
    return component == nullptr ? 0.0f : as_transform_component(component)->scaleX();
}

void rive_rs_transform_component_set_scale_x(rive_rs_transform_component* component,
                                             float value)
{
    if (component != nullptr)
    {
        as_transform_component(component)->scaleX(value);
    }
}

float rive_rs_transform_component_scale_y(const rive_rs_transform_component* component)
{
    return component == nullptr ? 0.0f : as_transform_component(component)->scaleY();
}

void rive_rs_transform_component_set_scale_y(rive_rs_transform_component* component,
                                             float value)
{
    if (component != nullptr)
    {
        as_transform_component(component)->scaleY(value);
    }
}

float rive_rs_transform_component_rotation(const rive_rs_transform_component* component)
{
    return component == nullptr ? 0.0f : as_transform_component(component)->rotation();
}

void rive_rs_transform_component_set_rotation(rive_rs_transform_component* component,
                                              float value)
{
    if (component != nullptr)
    {
        as_transform_component(component)->rotation(value);
    }
}

rive_rs_status rive_rs_transform_component_world_transform(
    const rive_rs_transform_component* component,
    rive_rs_mat2d* out_matrix)
{
    if (component == nullptr || out_matrix == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    copy_mat2d(as_transform_component(component)->worldTransform(), out_matrix);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_transform_component_parent_world_transform(
    const rive_rs_transform_component* component,
    rive_rs_mat2d* out_matrix)
{
    if (component == nullptr || out_matrix == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    copy_mat2d(rive::getParentWorld(*as_transform_component(component)), out_matrix);
    return RIVE_RS_STATUS_OK;
}

float rive_rs_node_x(const rive_rs_node* node)
{
    return node == nullptr ? 0.0f : as_node(node)->x();
}

void rive_rs_node_set_x(rive_rs_node* node, float value)
{
    if (node != nullptr)
    {
        as_node(node)->x(value);
    }
}

float rive_rs_node_y(const rive_rs_node* node)
{
    return node == nullptr ? 0.0f : as_node(node)->y();
}

void rive_rs_node_set_y(rive_rs_node* node, float value)
{
    if (node != nullptr)
    {
        as_node(node)->y(value);
    }
}

float rive_rs_bone_length(const rive_rs_bone* bone)
{
    return bone == nullptr ? 0.0f : as_bone(bone)->length();
}

void rive_rs_bone_set_length(rive_rs_bone* bone, float value)
{
    if (bone != nullptr)
    {
        as_bone(bone)->length(value);
    }
}

float rive_rs_root_bone_x(const rive_rs_root_bone* root_bone)
{
    return root_bone == nullptr ? 0.0f : as_root_bone(root_bone)->x();
}

void rive_rs_root_bone_set_x(rive_rs_root_bone* root_bone, float value)
{
    if (root_bone != nullptr)
    {
        as_root_bone(root_bone)->x(value);
    }
}

float rive_rs_root_bone_y(const rive_rs_root_bone* root_bone)
{
    return root_bone == nullptr ? 0.0f : as_root_bone(root_bone)->y();
}

void rive_rs_root_bone_set_y(rive_rs_root_bone* root_bone, float value)
{
    if (root_bone != nullptr)
    {
        as_root_bone(root_bone)->y(value);
    }
}

rive_rs_str_view rive_rs_text_value_run_name(const rive_rs_text_value_run* text_value_run)
{
    return text_value_run == nullptr ? kEmptyStrView
                                     : to_str_view(as_text_value_run(text_value_run)->name());
}

rive_rs_str_view rive_rs_text_value_run_text(const rive_rs_text_value_run* text_value_run)
{
    return text_value_run == nullptr ? kEmptyStrView
                                     : to_str_view(as_text_value_run(text_value_run)->text());
}

rive_rs_status rive_rs_text_value_run_set_text(rive_rs_text_value_run* text_value_run,
                                               rive_rs_str_view text)
{
    if (text_value_run == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    as_text_value_run(text_value_run)->text(from_str_view(text));
    return RIVE_RS_STATUS_OK;
}

void rive_rs_flattened_path_delete(rive_rs_flattened_path* path)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    delete as_flattened_path(path);
#else
    (void)path;
#endif
}

size_t rive_rs_flattened_path_length(const rive_rs_flattened_path* path)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    return path == nullptr ? 0 : as_flattened_path(path)->vertices().size();
#else
    (void)path;
    return 0;
#endif
}

rive_rs_status rive_rs_flattened_path_is_cubic(const rive_rs_flattened_path* path,
                                               size_t index,
                                               bool* out_is_cubic)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_is_cubic == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_is_cubic = vertices[index]->is<rive::CubicVertex>();
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_is_cubic;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_x(const rive_rs_flattened_path* path,
                                        size_t index,
                                        float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_value = vertices[index]->x();
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_y(const rive_rs_flattened_path* path,
                                        size_t index,
                                        float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_value = vertices[index]->y();
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_in_x(const rive_rs_flattened_path* path,
                                           size_t index,
                                           float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }
    if (!vertices[index]->is<rive::CubicVertex>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_value = vertices[index]->as<rive::CubicVertex>()->renderIn().x;
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_in_y(const rive_rs_flattened_path* path,
                                           size_t index,
                                           float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }
    if (!vertices[index]->is<rive::CubicVertex>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_value = vertices[index]->as<rive::CubicVertex>()->renderIn().y;
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_out_x(const rive_rs_flattened_path* path,
                                            size_t index,
                                            float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }
    if (!vertices[index]->is<rive::CubicVertex>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_value = vertices[index]->as<rive::CubicVertex>()->renderOut().x;
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_flattened_path_out_y(const rive_rs_flattened_path* path,
                                            size_t index,
                                            float* out_value)
{
#ifdef ENABLE_QUERY_FLAT_VERTICES
    if (path == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto& vertices = as_flattened_path(path)->vertices();
    if (index >= vertices.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }
    if (!vertices[index]->is<rive::CubicVertex>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_value = vertices[index]->as<rive::CubicVertex>()->renderOut().y;
    return RIVE_RS_STATUS_OK;
#else
    (void)path;
    (void)index;
    (void)out_value;
    return RIVE_RS_STATUS_UNSUPPORTED;
#endif
}

rive_rs_status rive_rs_linear_animation_instance_new(
    rive_rs_linear_animation* animation,
    rive_rs_artboard* artboard,
    rive_rs_linear_animation_instance** out_instance)
{
    if (animation == nullptr || artboard == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto* instance = new (std::nothrow)
        rive::LinearAnimationInstance(as_linear_animation(animation), as_artboard(artboard));
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_instance = reinterpret_cast<rive_rs_linear_animation_instance*>(instance);
    return RIVE_RS_STATUS_OK;
}

rive_rs_str_view rive_rs_linear_animation_name(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? kEmptyStrView
                                : to_str_view(as_linear_animation(animation)->name());
}

uint32_t rive_rs_linear_animation_duration(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0 : as_linear_animation(animation)->duration();
}

uint32_t rive_rs_linear_animation_fps(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0 : as_linear_animation(animation)->fps();
}

uint32_t rive_rs_linear_animation_work_start(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0 : as_linear_animation(animation)->workStart();
}

uint32_t rive_rs_linear_animation_work_end(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0 : as_linear_animation(animation)->workEnd();
}

bool rive_rs_linear_animation_enable_work_area(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? false : as_linear_animation(animation)->enableWorkArea();
}

uint32_t rive_rs_linear_animation_loop_value(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0 : as_linear_animation(animation)->loopValue();
}

float rive_rs_linear_animation_speed(const rive_rs_linear_animation* animation)
{
    return animation == nullptr ? 0.0f : as_linear_animation(animation)->speed();
}

rive_rs_status rive_rs_linear_animation_apply(const rive_rs_linear_animation* animation,
                                              rive_rs_artboard* artboard,
                                              float time,
                                              float mix)
{
    if (animation == nullptr || artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    as_linear_animation(animation)->apply(as_artboard(artboard), time, mix);
    return RIVE_RS_STATUS_OK;
}

void rive_rs_linear_animation_instance_delete(rive_rs_linear_animation_instance* instance)
{
    delete as_linear_animation_instance(instance);
}

rive_rs_status rive_rs_linear_animation_instance_advance(
    rive_rs_linear_animation_instance* instance,
    float seconds,
    bool* out_looped)
{
    if (instance == nullptr || out_looped == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)as_linear_animation_instance(instance)->advance(seconds);
    *out_looped = as_linear_animation_instance(instance)->didLoop();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_linear_animation_instance_apply(
    rive_rs_linear_animation_instance* instance,
    rive_rs_artboard* artboard,
    float mix)
{
    if (instance == nullptr || artboard == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)artboard;
    as_linear_animation_instance(instance)->apply(mix);
    return RIVE_RS_STATUS_OK;
}

float rive_rs_linear_animation_instance_time(const rive_rs_linear_animation_instance* instance)
{
    return instance == nullptr ? 0.0f : as_linear_animation_instance(instance)->time();
}

void rive_rs_linear_animation_instance_set_time(rive_rs_linear_animation_instance* instance,
                                                float seconds)
{
    if (instance != nullptr)
    {
        as_linear_animation_instance(instance)->time(seconds);
    }
}

bool rive_rs_linear_animation_instance_did_loop(
    const rive_rs_linear_animation_instance* instance)
{
    return instance == nullptr ? false : as_linear_animation_instance(instance)->didLoop();
}

rive_rs_status rive_rs_state_machine_instance_new(
    rive_rs_state_machine* state_machine,
    rive_rs_artboard* artboard,
    rive_rs_state_machine_instance** out_instance)
{
    if (state_machine == nullptr || artboard == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto* instance = new (std::nothrow)
        rive::StateMachineInstance(as_state_machine(state_machine), as_artboard(artboard));
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_instance = reinterpret_cast<rive_rs_state_machine_instance*>(instance);
    return RIVE_RS_STATUS_OK;
}

rive_rs_str_view rive_rs_state_machine_name(const rive_rs_state_machine* state_machine)
{
    return state_machine == nullptr ? kEmptyStrView
                                    : to_str_view(as_state_machine(state_machine)->name());
}

void rive_rs_state_machine_instance_delete(rive_rs_state_machine_instance* instance)
{
    delete as_state_machine_instance(instance);
}

rive_rs_status rive_rs_state_machine_instance_advance(
    rive_rs_state_machine_instance* instance,
    float seconds,
    bool* out_changed)
{
    if (instance == nullptr || out_changed == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_changed = as_state_machine_instance(instance)->advance(seconds);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_state_machine_instance_advance_and_apply(
    rive_rs_state_machine_instance* instance,
    float seconds,
    bool* out_changed)
{
    if (instance == nullptr || out_changed == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_changed = as_state_machine_instance(instance)->advanceAndApply(seconds);
    return RIVE_RS_STATUS_OK;
}

size_t rive_rs_state_machine_input_count(const rive_rs_state_machine_instance* instance)
{
    return instance == nullptr ? 0 : as_state_machine_instance(instance)->inputCount();
}

rive_rs_status rive_rs_state_machine_input_at(rive_rs_state_machine_instance* instance,
                                              size_t index,
                                              rive_rs_smi_input** out_input)
{
    if (instance == nullptr || out_input == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_input = nullptr;
    auto* input = as_state_machine_instance(instance)->input(index);
    if (input == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_input = reinterpret_cast<rive_rs_smi_input*>(input);
    return RIVE_RS_STATUS_OK;
}

rive_rs_smi_input_type rive_rs_smi_input_type_of(const rive_rs_smi_input* input)
{
    if (input == nullptr)
    {
        return static_cast<rive_rs_smi_input_type>(0);
    }

    switch (as_smi_input(input)->inputCoreType())
    {
        case rive::StateMachineBoolBase::typeKey:
            return RIVE_RS_SMI_INPUT_BOOL;
        case rive::StateMachineNumberBase::typeKey:
            return RIVE_RS_SMI_INPUT_NUMBER;
        case rive::StateMachineTriggerBase::typeKey:
            return RIVE_RS_SMI_INPUT_TRIGGER;
        default:
            return static_cast<rive_rs_smi_input_type>(0);
    }
}

rive_rs_str_view rive_rs_smi_input_name(const rive_rs_smi_input* input)
{
    return input == nullptr ? kEmptyStrView : to_str_view(as_smi_input(input)->name());
}

rive_rs_status rive_rs_smi_input_as_bool(rive_rs_smi_input* input,
                                         rive_rs_smi_bool** out_bool)
{
    if (input == nullptr || out_bool == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_bool = nullptr;
    if (as_smi_input(input)->inputCoreType() != rive::StateMachineBoolBase::typeKey)
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_bool = reinterpret_cast<rive_rs_smi_bool*>(as_smi_input(input));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_smi_input_as_number(rive_rs_smi_input* input,
                                           rive_rs_smi_number** out_number)
{
    if (input == nullptr || out_number == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_number = nullptr;
    if (as_smi_input(input)->inputCoreType() != rive::StateMachineNumberBase::typeKey)
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_number = reinterpret_cast<rive_rs_smi_number*>(as_smi_input(input));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_smi_input_as_trigger(rive_rs_smi_input* input,
                                            rive_rs_smi_trigger** out_trigger)
{
    if (input == nullptr || out_trigger == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_trigger = nullptr;
    if (as_smi_input(input)->inputCoreType() != rive::StateMachineTriggerBase::typeKey)
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    *out_trigger = reinterpret_cast<rive_rs_smi_trigger*>(as_smi_input(input));
    return RIVE_RS_STATUS_OK;
}

bool rive_rs_smi_bool_get(const rive_rs_smi_bool* input)
{
    return input == nullptr ? false : as_smi_bool(input)->value();
}

void rive_rs_smi_bool_set(rive_rs_smi_bool* input, bool value)
{
    if (input != nullptr)
    {
        as_smi_bool(input)->value(value);
    }
}

float rive_rs_smi_number_get(const rive_rs_smi_number* input)
{
    return input == nullptr ? 0.0f : as_smi_number(input)->value();
}

void rive_rs_smi_number_set(rive_rs_smi_number* input, float value)
{
    if (input != nullptr)
    {
        as_smi_number(input)->value(value);
    }
}

void rive_rs_smi_trigger_fire(rive_rs_smi_trigger* input)
{
    if (input != nullptr)
    {
        as_smi_trigger(input)->fire();
    }
}

rive_rs_status rive_rs_state_machine_instance_pointer_down(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)as_state_machine_instance(instance)->pointerDown(to_runtime_vec2(point), pointer_id);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_state_machine_instance_pointer_move(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)as_state_machine_instance(instance)->pointerMove(to_runtime_vec2(point), 0.0f, pointer_id);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_state_machine_instance_pointer_up(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)as_state_machine_instance(instance)->pointerUp(to_runtime_vec2(point), pointer_id);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_state_machine_instance_pointer_exit(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    (void)as_state_machine_instance(instance)->pointerExit(to_runtime_vec2(point), pointer_id);
    return RIVE_RS_STATUS_OK;
}

bool rive_rs_state_machine_instance_has_listeners(
    const rive_rs_state_machine_instance* instance)
{
    return instance == nullptr ? false
                               : const_cast<rive::StateMachineInstance*>(
                                     as_state_machine_instance(instance))
                                     ->hasListeners();
}

bool rive_rs_state_machine_instance_has_any_listener(
    const rive_rs_state_machine_instance* instance)
{
    return instance == nullptr
               ? false
               : has_any_listener(
                     const_cast<rive::StateMachineInstance*>(
                         as_state_machine_instance(instance)));
}

size_t rive_rs_state_machine_reported_event_count(
    const rive_rs_state_machine_instance* instance)
{
    return instance == nullptr ? 0 : as_state_machine_instance(instance)->reportedEventCount();
}

rive_rs_status rive_rs_state_machine_reported_event_at(
    const rive_rs_state_machine_instance* instance,
    size_t index,
    rive_rs_event_info* out_event,
    float* out_delay_seconds)
{
    if (instance == nullptr || out_event == nullptr || out_delay_seconds == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    if (index >= as_state_machine_instance(instance)->reportedEventCount())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    const auto report = as_state_machine_instance(instance)->reportedEventAt(index);
    auto* event = report.event();
    if (event == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    fill_event_info(event, out_event);
    *out_delay_seconds = report.secondsDelay();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_state_machine_reported_event_property_at(
    const rive_rs_state_machine_instance* instance,
    size_t reported_event_index,
    size_t property_index,
    rive_rs_event_property_info* out_property)
{
    if (instance == nullptr || out_property == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    if (reported_event_index >= as_state_machine_instance(instance)->reportedEventCount())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    const auto report =
        as_state_machine_instance(instance)->reportedEventAt(reported_event_index);
    auto* event = report.event();
    if (event == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto* property = custom_property_at(event, property_index);
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    return fill_event_property_info(property, out_property);
}

size_t rive_rs_state_machine_state_changed_count(
    const rive_rs_state_machine_instance* instance)
{
    return instance == nullptr ? 0 : as_state_machine_instance(instance)->stateChangedCount();
}

rive_rs_status rive_rs_state_machine_state_changed_name_at(
    const rive_rs_state_machine_instance* instance,
    size_t index,
    rive_rs_str_view* out_name)
{
    if (instance == nullptr || out_name == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_name = kEmptyStrView;
    const auto* state = as_state_machine_instance(instance)->stateChangedByIndex(index);
    if (state == nullptr)
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    switch (state->coreType())
    {
        case rive::AnimationState::typeKey:
        {
            auto* animation_state = state->as<rive::AnimationState>();
            auto* animation = animation_state->animation();
            if (animation == nullptr)
            {
                return RIVE_RS_STATUS_NOT_FOUND;
            }
            *out_name = to_str_view(animation->name());
            return RIVE_RS_STATUS_OK;
        }
        case rive::EntryState::typeKey:
            *out_name = to_str_view("entry");
            return RIVE_RS_STATUS_OK;
        case rive::ExitState::typeKey:
            *out_name = to_str_view("exit");
            return RIVE_RS_STATUS_OK;
        case rive::AnyState::typeKey:
            *out_name = to_str_view("any");
            return RIVE_RS_STATUS_OK;
        default:
            *out_name = to_str_view("unknown");
            return RIVE_RS_STATUS_OK;
    }
}

rive_rs_status rive_rs_state_machine_instance_bind_view_model_instance(
    rive_rs_state_machine_instance* instance,
    rive_rs_view_model_instance* view_model_instance)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto runtime_instance = view_model_instance == nullptr
                                ? rive::rcp<rive::ViewModelInstance>(nullptr)
                                : as_view_model_instance(view_model_instance)->instance();
    as_state_machine_instance(instance)->bindViewModelInstance(runtime_instance);
    return RIVE_RS_STATUS_OK;
}

void rive_rs_view_model_ref(rive_rs_view_model* view_model)
{
    if (view_model != nullptr)
    {
        as_view_model(view_model)->ref();
    }
}

void rive_rs_view_model_unref(rive_rs_view_model* view_model)
{
    if (view_model != nullptr)
    {
        as_view_model(view_model)->unref();
    }
}

rive_rs_str_view rive_rs_view_model_name(const rive_rs_view_model* view_model)
{
    return view_model == nullptr ? kEmptyStrView : to_str_view(as_view_model(view_model)->name());
}

size_t rive_rs_view_model_property_count(const rive_rs_view_model* view_model)
{
    return view_model == nullptr ? 0 : as_view_model(view_model)->propertyCount();
}

size_t rive_rs_view_model_instance_count(const rive_rs_view_model* view_model)
{
    return view_model == nullptr ? 0 : as_view_model(view_model)->instanceCount();
}

rive_rs_status rive_rs_view_model_property_at(const rive_rs_view_model* view_model,
                                              size_t index,
                                              rive_rs_property_info* out_property)
{
    if (view_model == nullptr || out_property == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto properties = as_view_model(view_model)->properties();
    if (index >= properties.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    out_property->data_type = to_abi_data_type(properties[index].type);
    out_property->name = to_scratch_view(properties[index].name);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_name_at(const rive_rs_view_model* view_model,
                                                   size_t index,
                                                   rive_rs_str_view* out_name)
{
    if (view_model == nullptr || out_name == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_name = kEmptyStrView;
    auto names = as_view_model(view_model)->instanceNames();
    if (index >= names.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_name = to_scratch_view(names[index]);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_by_index(
    const rive_rs_view_model* view_model,
    size_t index,
    rive_rs_view_model_instance** out_instance)
{
    if (view_model == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto instance = as_view_model(view_model)->createInstanceFromIndex(index);
    if (instance == nullptr)
    {
        return index >= as_view_model(view_model)->instanceCount() ? RIVE_RS_STATUS_OUT_OF_RANGE
                                                                    : RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_instance = reinterpret_cast<rive_rs_view_model_instance*>(instance.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_by_name(
    const rive_rs_view_model* view_model,
    rive_rs_str_view name,
    rive_rs_view_model_instance** out_instance)
{
    if (view_model == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto instance = as_view_model(view_model)->createInstanceFromName(from_str_view(name));
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_instance = reinterpret_cast<rive_rs_view_model_instance*>(instance.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_default_instance(
    const rive_rs_view_model* view_model,
    rive_rs_view_model_instance** out_instance)
{
    if (view_model == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto instance = as_view_model(view_model)->createDefaultInstance();
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_instance = reinterpret_cast<rive_rs_view_model_instance*>(instance.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_new_instance(const rive_rs_view_model* view_model,
                                               rive_rs_view_model_instance** out_instance)
{
    if (view_model == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto instance = as_view_model(view_model)->createInstance();
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_RUNTIME_ERROR;
    }

    *out_instance = reinterpret_cast<rive_rs_view_model_instance*>(instance.release());
    return RIVE_RS_STATUS_OK;
}

void rive_rs_view_model_instance_ref(rive_rs_view_model_instance* instance)
{
    if (instance != nullptr)
    {
        as_view_model_instance(instance)->ref();
    }
}

void rive_rs_view_model_instance_unref(rive_rs_view_model_instance* instance)
{
    if (instance != nullptr)
    {
        as_view_model_instance(instance)->unref();
    }
}

size_t rive_rs_view_model_instance_property_count(
    const rive_rs_view_model_instance* instance)
{
    return instance == nullptr ? 0 : as_view_model_instance(instance)->propertyCount();
}

rive_rs_status rive_rs_view_model_instance_property_at(
    const rive_rs_view_model_instance* instance,
    size_t index,
    rive_rs_property_info* out_property)
{
    if (instance == nullptr || out_property == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto properties = as_view_model_instance(instance)->properties();
    if (index >= properties.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    out_property->data_type = to_abi_data_type(properties[index].type);
    out_property->name = to_scratch_view(properties[index].name);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_number(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    float* out_value)
{
    if (instance == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyNumber(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_value = property->value();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_number(rive_rs_view_model_instance* instance,
                                                      rive_rs_str_view path,
                                                      float value)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyNumber(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(value);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_string(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_str_view* out_value)
{
    if (instance == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_value = kEmptyStrView;
    auto* property = as_view_model_instance(instance)->propertyString(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_value = to_str_view(property->value());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_string(rive_rs_view_model_instance* instance,
                                                      rive_rs_str_view path,
                                                      rive_rs_str_view value)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyString(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(from_str_view(value));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_boolean(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    bool* out_value)
{
    if (instance == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyBoolean(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_value = property->value();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_boolean(rive_rs_view_model_instance* instance,
                                                       rive_rs_str_view path,
                                                       bool value)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyBoolean(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(value);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_color(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    int32_t* out_argb)
{
    if (instance == nullptr || out_argb == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyColor(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_argb = property->value();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_color(rive_rs_view_model_instance* instance,
                                                     rive_rs_str_view path,
                                                     int32_t argb)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyColor(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(argb);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_enum(const rive_rs_view_model_instance* instance,
                                                    rive_rs_str_view path,
                                                    rive_rs_str_view* out_value)
{
    if (instance == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_value = kEmptyStrView;
    auto* property = as_view_model_instance(instance)->propertyEnum(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_value = to_scratch_view(property->value());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_enum(rive_rs_view_model_instance* instance,
                                                    rive_rs_str_view path,
                                                    rive_rs_str_view value)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyEnum(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(from_str_view(value));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_enum_index(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    uint32_t* out_index)
{
    if (instance == nullptr || out_index == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyEnum(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_index = property->valueIndex();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_enum_index(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    uint32_t index)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyEnum(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    auto values = property->values();
    if (index >= values.size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    property->valueIndex(index);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_fire_trigger(rive_rs_view_model_instance* instance,
                                                        rive_rs_str_view path)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyTrigger(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->trigger();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_get_view_model(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance** out_instance)
{
    if (instance == nullptr || out_instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_instance = nullptr;
    auto nested_instance = as_view_model_instance(instance)->propertyViewModel(from_str_view(path));
    if (nested_instance == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_instance = reinterpret_cast<rive_rs_view_model_instance*>(nested_instance.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_replace_view_model(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value)
{
    if (instance == nullptr || value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    if (!as_view_model_instance(instance)->replaceViewModel(from_str_view(path),
                                                            as_view_model_instance(value)))
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_property_has_changed(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    bool* out_changed)
{
    if (instance == nullptr || out_changed == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->property(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_changed = property->hasChanged();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_clear_property_changes(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->property(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->clearChanges();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_size(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t* out_size)
{
    if (instance == nullptr || out_size == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_size = list_property->size();
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_instance_at(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t index,
    rive_rs_view_model_instance** out_value)
{
    if (instance == nullptr || out_value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_value = nullptr;
    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }
    if (index >= list_property->size() ||
        index > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    auto value = list_property->instanceAt(static_cast<int>(index));
    if (value == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    *out_value = reinterpret_cast<rive_rs_view_model_instance*>(value.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_add_instance(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value)
{
    if (instance == nullptr || value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    list_property->addInstance(as_view_model_instance(value));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_add_instance_at(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value,
    size_t index,
    bool* out_added)
{
    if (instance == nullptr || value == nullptr || out_added == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_added = false;
    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    if (index > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    *out_added =
        list_property->addInstanceAt(as_view_model_instance(value), static_cast<int>(index));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_remove_instance(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value)
{
    if (instance == nullptr || value == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    list_property->removeInstance(as_view_model_instance(value));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_remove_instance_at(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t index)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    if (index >= list_property->size() ||
        index > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    list_property->removeInstanceAt(static_cast<int>(index));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_list_swap(rive_rs_view_model_instance* instance,
                                                     rive_rs_str_view path,
                                                     uint32_t a,
                                                     uint32_t b)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* list_property = as_view_model_instance(instance)->propertyList(from_str_view(path));
    if (list_property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    if (a >= list_property->size() || b >= list_property->size())
    {
        return RIVE_RS_STATUS_OUT_OF_RANGE;
    }

    list_property->swap(a, b);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_artboard(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_bindable_artboard* value)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyArtboard(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->value(value == nullptr
                        ? rive::rcp<rive::BindableArtboard>(nullptr)
                        : rive::ref_rcp(as_bindable_artboard(value)));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_view_model_instance_set_artboard_view_model(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* view_model_instance)
{
    if (instance == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* property = as_view_model_instance(instance)->propertyArtboard(from_str_view(path));
    if (property == nullptr)
    {
        return RIVE_RS_STATUS_NOT_FOUND;
    }

    property->viewModelInstance(view_model_instance == nullptr
                                    ? rive::rcp<rive::ViewModelInstance>(nullptr)
                                    : as_view_model_instance(view_model_instance)->instance());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_compute_alignment(rive_rs_fit fit,
                                         rive_rs_alignment alignment,
                                         const rive_rs_aabb* source,
                                         const rive_rs_aabb* destination,
                                         float scale_factor,
                                         rive_rs_mat2d* out_matrix)
{
    if (source == nullptr || destination == nullptr || out_matrix == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    rive::Fit runtime_fit;
    rive::Alignment runtime_alignment;
    if (!to_runtime_fit(fit, &runtime_fit) ||
        !to_runtime_alignment(alignment, &runtime_alignment))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto matrix = rive::computeAlignment(runtime_fit,
                                         runtime_alignment,
                                         to_runtime_aabb(*source),
                                         to_runtime_aabb(*destination),
                                         scale_factor);
    copy_mat2d(matrix, out_matrix);
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_map_xy(const rive_rs_mat2d* matrix,
                              rive_rs_vec2 point,
                              rive_rs_vec2* out_point)
{
    if (matrix == nullptr || out_point == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    const auto mapped = to_mat2d(*matrix) * to_runtime_vec2(point);
    out_point->x = mapped.x;
    out_point->y = mapped.y;
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_decode_audio(rive_rs_factory* factory,
                                    rive_rs_bytes_view bytes,
                                    rive_rs_audio_source** out_audio)
{
    if (factory == nullptr || out_audio == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_audio = nullptr;
    if (as_factory(factory) == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (invalid_bytes(bytes))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto audio = as_factory(factory)->decodeAudio(to_span(bytes));
    if (audio == nullptr)
    {
        return RIVE_RS_STATUS_DECODE_ERROR;
    }

    *out_audio = reinterpret_cast<rive_rs_audio_source*>(audio.release());
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_decode_font(rive_rs_factory* factory,
                                   rive_rs_bytes_view bytes,
                                   rive_rs_font** out_font)
{
    if (factory == nullptr || out_font == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    *out_font = nullptr;
    if (as_factory(factory) == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (invalid_bytes(bytes))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto font = as_factory(factory)->decodeFont(to_span(bytes));
    if (font == nullptr)
    {
        return RIVE_RS_STATUS_DECODE_ERROR;
    }

    *out_font = reinterpret_cast<rive_rs_font*>(font.release());
    return RIVE_RS_STATUS_OK;
}

void rive_rs_audio_source_unref(rive_rs_audio_source* audio)
{
    if (audio != nullptr)
    {
        as_audio_source(audio)->unref();
    }
}

void rive_rs_font_unref(rive_rs_font* font)
{
    if (font != nullptr)
    {
        as_font(font)->unref();
    }
}

rive_rs_file_asset* rive_rs_ptr_to_file_asset(uintptr_t pointer)
{
    return reinterpret_cast<rive_rs_file_asset*>(pointer);
}

rive_rs_file_asset* rive_rs_ptr_to_audio_asset(uintptr_t pointer)
{
    return reinterpret_cast<rive_rs_file_asset*>(pointer);
}

rive_rs_file_asset* rive_rs_ptr_to_image_asset(uintptr_t pointer)
{
    return reinterpret_cast<rive_rs_file_asset*>(pointer);
}

rive_rs_file_asset* rive_rs_ptr_to_font_asset(uintptr_t pointer)
{
    return reinterpret_cast<rive_rs_file_asset*>(pointer);
}

rive_rs_str_view rive_rs_file_asset_name(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? kEmptyStrView : to_str_view(as_file_asset(asset)->name());
}

rive_rs_str_view rive_rs_file_asset_cdn_base_url(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? kEmptyStrView : to_str_view(as_file_asset(asset)->cdnBaseUrl());
}

rive_rs_str_view rive_rs_file_asset_file_extension(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? kEmptyStrView
                            : to_scratch_view(as_file_asset(asset)->fileExtension());
}

rive_rs_str_view rive_rs_file_asset_unique_filename(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? kEmptyStrView
                            : to_scratch_view(as_file_asset(asset)->uniqueFilename());
}

bool rive_rs_file_asset_is_audio(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? false : as_file_asset(asset)->is<rive::AudioAsset>();
}

bool rive_rs_file_asset_is_image(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? false : as_file_asset(asset)->is<rive::ImageAsset>();
}

bool rive_rs_file_asset_is_font(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? false : as_file_asset(asset)->is<rive::FontAsset>();
}

rive_rs_str_view rive_rs_file_asset_cdn_uuid(const rive_rs_file_asset* asset)
{
    return asset == nullptr ? kEmptyStrView : to_scratch_view(as_file_asset(asset)->cdnUuidStr());
}

rive_rs_status rive_rs_file_asset_decode(rive_rs_factory* factory,
                                         rive_rs_file_asset* asset,
                                         rive_rs_bytes_view bytes)
{
    if (factory == nullptr || asset == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (as_factory(factory) == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    if (invalid_bytes(bytes))
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto data = to_simple_array(bytes);
    if (!as_file_asset(asset)->decode(data, as_factory(factory)))
    {
        return RIVE_RS_STATUS_DECODE_ERROR;
    }

    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_audio_asset_set_audio_source(rive_rs_file_asset* asset,
                                                    rive_rs_audio_source* audio)
{
    if (asset == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* file_asset = as_file_asset(asset);
    if (!file_asset->is<rive::AudioAsset>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto* audio_asset = file_asset->as<rive::AudioAsset>();
    audio_asset->audioSource(audio == nullptr ? rive::rcp<rive::AudioSource>(nullptr)
                                              : rive::ref_rcp(as_audio_source(audio)));
    return RIVE_RS_STATUS_OK;
}

rive_rs_status rive_rs_font_asset_set_font(rive_rs_file_asset* asset,
                                           rive_rs_font* font)
{
    if (asset == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }

    auto* file_asset = as_file_asset(asset);
    if (!file_asset->is<rive::FontAsset>())
    {
        return RIVE_RS_STATUS_INVALID_ARGUMENT;
    }

    auto* font_asset = file_asset->as<rive::FontAsset>();
    font_asset->font(font == nullptr ? rive::rcp<rive::Font>(nullptr)
                                     : rive::ref_rcp(as_font(font)));
    return RIVE_RS_STATUS_OK;
}
} // extern "C"
