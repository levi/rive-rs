#ifndef RIVE_RS_ABI_H
#define RIVE_RS_ABI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
  #if defined(RIVE_RS_BUILD_DLL)
    #define RIVE_RS_API __declspec(dllexport)
  #elif defined(RIVE_RS_USE_DLL)
    #define RIVE_RS_API __declspec(dllimport)
  #else
    #define RIVE_RS_API
  #endif
#else
  #define RIVE_RS_API
#endif

typedef struct rive_rs_factory rive_rs_factory;
typedef struct rive_rs_file rive_rs_file;
typedef struct rive_rs_artboard rive_rs_artboard;
typedef struct rive_rs_bindable_artboard rive_rs_bindable_artboard;
typedef struct rive_rs_view_model rive_rs_view_model;
typedef struct rive_rs_view_model_instance rive_rs_view_model_instance;
typedef struct rive_rs_transform_component rive_rs_transform_component;
typedef struct rive_rs_node rive_rs_node;
typedef struct rive_rs_bone rive_rs_bone;
typedef struct rive_rs_root_bone rive_rs_root_bone;
typedef struct rive_rs_text_value_run rive_rs_text_value_run;
typedef struct rive_rs_flattened_path rive_rs_flattened_path;

typedef struct rive_rs_renderer rive_rs_renderer;
typedef struct rive_rs_webgl2_renderer rive_rs_webgl2_renderer;
typedef struct rive_rs_webgpu_renderer rive_rs_webgpu_renderer;

typedef struct rive_rs_linear_animation rive_rs_linear_animation;
typedef struct rive_rs_linear_animation_instance rive_rs_linear_animation_instance;
typedef struct rive_rs_state_machine rive_rs_state_machine;
typedef struct rive_rs_state_machine_instance rive_rs_state_machine_instance;
typedef struct rive_rs_smi_input rive_rs_smi_input;
typedef struct rive_rs_smi_bool rive_rs_smi_bool;
typedef struct rive_rs_smi_number rive_rs_smi_number;
typedef struct rive_rs_smi_trigger rive_rs_smi_trigger;

typedef struct rive_rs_file_asset rive_rs_file_asset;
typedef struct rive_rs_audio_source rive_rs_audio_source;
typedef struct rive_rs_font rive_rs_font;
typedef struct rive_rs_render_image rive_rs_render_image;

typedef enum rive_rs_status {
  RIVE_RS_STATUS_OK = 0,
  RIVE_RS_STATUS_NULL = 1,
  RIVE_RS_STATUS_INVALID_ARGUMENT = 2,
  RIVE_RS_STATUS_NOT_FOUND = 3,
  RIVE_RS_STATUS_OUT_OF_RANGE = 4,
  RIVE_RS_STATUS_UNSUPPORTED = 5,
  RIVE_RS_STATUS_DECODE_ERROR = 6,
  RIVE_RS_STATUS_RUNTIME_ERROR = 7,
} rive_rs_status;

typedef enum rive_rs_fit {
  RIVE_RS_FIT_FILL = 0,
  RIVE_RS_FIT_CONTAIN = 1,
  RIVE_RS_FIT_COVER = 2,
  RIVE_RS_FIT_FIT_WIDTH = 3,
  RIVE_RS_FIT_FIT_HEIGHT = 4,
  RIVE_RS_FIT_NONE = 5,
  RIVE_RS_FIT_SCALE_DOWN = 6,
  RIVE_RS_FIT_LAYOUT = 7,
} rive_rs_fit;

typedef enum rive_rs_alignment {
  RIVE_RS_ALIGNMENT_TOP_LEFT = 0,
  RIVE_RS_ALIGNMENT_TOP_CENTER = 1,
  RIVE_RS_ALIGNMENT_TOP_RIGHT = 2,
  RIVE_RS_ALIGNMENT_CENTER_LEFT = 3,
  RIVE_RS_ALIGNMENT_CENTER = 4,
  RIVE_RS_ALIGNMENT_CENTER_RIGHT = 5,
  RIVE_RS_ALIGNMENT_BOTTOM_LEFT = 6,
  RIVE_RS_ALIGNMENT_BOTTOM_CENTER = 7,
  RIVE_RS_ALIGNMENT_BOTTOM_RIGHT = 8,
} rive_rs_alignment;

typedef enum rive_rs_smi_input_type {
  RIVE_RS_SMI_INPUT_BOOL = 59,
  RIVE_RS_SMI_INPUT_NUMBER = 56,
  RIVE_RS_SMI_INPUT_TRIGGER = 58,
} rive_rs_smi_input_type;

typedef enum rive_rs_data_type {
  RIVE_RS_DATA_TYPE_NONE = 0,
  RIVE_RS_DATA_TYPE_STRING = 1,
  RIVE_RS_DATA_TYPE_NUMBER = 2,
  RIVE_RS_DATA_TYPE_BOOLEAN = 3,
  RIVE_RS_DATA_TYPE_COLOR = 4,
  RIVE_RS_DATA_TYPE_LIST = 5,
  RIVE_RS_DATA_TYPE_ENUM = 6,
  RIVE_RS_DATA_TYPE_TRIGGER = 7,
  RIVE_RS_DATA_TYPE_VIEW_MODEL = 8,
  RIVE_RS_DATA_TYPE_INTEGER = 9,
  RIVE_RS_DATA_TYPE_LIST_INDEX = 10,
  RIVE_RS_DATA_TYPE_IMAGE = 11,
  RIVE_RS_DATA_TYPE_ARTBOARD = 12,
} rive_rs_data_type;

typedef enum rive_rs_event_property_type {
  RIVE_RS_EVENT_PROPERTY_BOOL = 1,
  RIVE_RS_EVENT_PROPERTY_NUMBER = 2,
  RIVE_RS_EVENT_PROPERTY_STRING = 3,
} rive_rs_event_property_type;

typedef struct rive_rs_vec2 {
  float x;
  float y;
} rive_rs_vec2;

typedef struct rive_rs_mat2d {
  float xx;
  float xy;
  float yx;
  float yy;
  float tx;
  float ty;
} rive_rs_mat2d;

typedef struct rive_rs_aabb {
  float min_x;
  float min_y;
  float max_x;
  float max_y;
} rive_rs_aabb;

typedef struct rive_rs_bytes_view {
  const uint8_t* ptr;
  size_t len;
} rive_rs_bytes_view;

typedef struct rive_rs_str_view {
  const char* ptr;
  size_t len;
} rive_rs_str_view;

typedef struct rive_rs_property_info {
  rive_rs_str_view name;
  rive_rs_data_type data_type;
} rive_rs_property_info;

typedef struct rive_rs_event_info {
  rive_rs_str_view name;
  uint32_t type;
  bool has_url;
  rive_rs_str_view url;
  bool has_target;
  rive_rs_str_view target;
  size_t property_count;
} rive_rs_event_info;

typedef struct rive_rs_event_property_info {
  rive_rs_str_view name;
  rive_rs_event_property_type value_type;
  bool bool_value;
  float number_value;
  rive_rs_str_view string_value;
} rive_rs_event_property_info;

typedef bool (*rive_rs_file_asset_loader_load_contents_fn)(
    void* user_data,
    rive_rs_file_asset* asset,
    rive_rs_bytes_view in_band_bytes,
    rive_rs_factory* factory);

typedef struct rive_rs_file_asset_loader_callbacks {
  rive_rs_file_asset_loader_load_contents_fn load_contents;
  void* user_data;
} rive_rs_file_asset_loader_callbacks;

RIVE_RS_API uint32_t rive_rs_abi_version(void);

RIVE_RS_API rive_rs_factory* rive_rs_factory_default(void);
RIVE_RS_API rive_rs_factory* rive_rs_factory_webgl2(void);
RIVE_RS_API rive_rs_factory* rive_rs_factory_webgpu(void);
RIVE_RS_API void rive_rs_factory_ref(rive_rs_factory* factory);
RIVE_RS_API void rive_rs_factory_unref(rive_rs_factory* factory);

RIVE_RS_API rive_rs_status rive_rs_load_file(
    rive_rs_factory* factory,
    rive_rs_bytes_view bytes,
    rive_rs_file** out_file);
RIVE_RS_API rive_rs_status rive_rs_load_file_with_asset_loader(
    rive_rs_factory* factory,
    rive_rs_bytes_view bytes,
    const rive_rs_file_asset_loader_callbacks* callbacks,
    rive_rs_file** out_file);
RIVE_RS_API void rive_rs_file_ref(rive_rs_file* file);
RIVE_RS_API void rive_rs_file_unref(rive_rs_file* file);

RIVE_RS_API size_t rive_rs_file_artboard_count(const rive_rs_file* file);
RIVE_RS_API rive_rs_status rive_rs_file_artboard_default(
    const rive_rs_file* file,
    rive_rs_artboard** out_artboard);
RIVE_RS_API rive_rs_status rive_rs_file_artboard_by_index(
    const rive_rs_file* file,
    size_t index,
    rive_rs_artboard** out_artboard);
RIVE_RS_API rive_rs_status rive_rs_file_artboard_by_name(
    const rive_rs_file* file,
    rive_rs_str_view name,
    rive_rs_artboard** out_artboard);
RIVE_RS_API size_t rive_rs_file_view_model_count(const rive_rs_file* file);
RIVE_RS_API rive_rs_status rive_rs_file_view_model_by_index(
    const rive_rs_file* file,
    size_t index,
    rive_rs_view_model** out_view_model);
RIVE_RS_API rive_rs_status rive_rs_file_view_model_by_name(
    const rive_rs_file* file,
    rive_rs_str_view name,
    rive_rs_view_model** out_view_model);
RIVE_RS_API rive_rs_status rive_rs_file_default_artboard_view_model(
    const rive_rs_file* file,
    rive_rs_artboard* artboard,
    rive_rs_view_model** out_view_model);
RIVE_RS_API rive_rs_status rive_rs_file_bindable_artboard_by_name(
    const rive_rs_file* file,
    rive_rs_str_view name,
    rive_rs_bindable_artboard** out_bindable_artboard);
RIVE_RS_API rive_rs_status rive_rs_file_bindable_artboard_default(
    const rive_rs_file* file,
    rive_rs_bindable_artboard** out_bindable_artboard);
RIVE_RS_API rive_rs_status rive_rs_file_bindable_artboard_from_artboard(
    const rive_rs_file* file,
    rive_rs_artboard* artboard,
    rive_rs_bindable_artboard** out_bindable_artboard);
RIVE_RS_API bool rive_rs_file_has_audio(const rive_rs_file* file);
RIVE_RS_API size_t rive_rs_file_enum_count(const rive_rs_file* file);
RIVE_RS_API rive_rs_status rive_rs_file_enum_name_at(
    const rive_rs_file* file,
    size_t enum_index,
    rive_rs_str_view* out_name);
RIVE_RS_API size_t rive_rs_file_enum_value_count(
    const rive_rs_file* file,
    size_t enum_index);
RIVE_RS_API rive_rs_status rive_rs_file_enum_value_name_at(
    const rive_rs_file* file,
    size_t enum_index,
    size_t value_index,
    rive_rs_str_view* out_name);

RIVE_RS_API void rive_rs_artboard_ref(rive_rs_artboard* artboard);
RIVE_RS_API void rive_rs_artboard_unref(rive_rs_artboard* artboard);
RIVE_RS_API rive_rs_status rive_rs_artboard_advance(
    rive_rs_artboard* artboard,
    float seconds,
    bool* out_changed);
RIVE_RS_API rive_rs_status rive_rs_artboard_draw(
    rive_rs_artboard* artboard,
    rive_rs_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_artboard_draw_webgl2(
    rive_rs_artboard* artboard,
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_artboard_draw_webgpu(
    rive_rs_artboard* artboard,
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API bool rive_rs_artboard_did_change(const rive_rs_artboard* artboard);
RIVE_RS_API rive_rs_str_view rive_rs_artboard_name(const rive_rs_artboard* artboard);
RIVE_RS_API rive_rs_aabb rive_rs_artboard_bounds(const rive_rs_artboard* artboard);
RIVE_RS_API float rive_rs_artboard_width(const rive_rs_artboard* artboard);
RIVE_RS_API float rive_rs_artboard_height(const rive_rs_artboard* artboard);
RIVE_RS_API void rive_rs_artboard_set_width(rive_rs_artboard* artboard, float width);
RIVE_RS_API void rive_rs_artboard_set_height(rive_rs_artboard* artboard, float height);
RIVE_RS_API bool rive_rs_artboard_frame_origin(const rive_rs_artboard* artboard);
RIVE_RS_API void rive_rs_artboard_set_frame_origin(
    rive_rs_artboard* artboard,
    bool frame_origin);
RIVE_RS_API bool rive_rs_artboard_has_audio(const rive_rs_artboard* artboard);
RIVE_RS_API float rive_rs_artboard_volume(const rive_rs_artboard* artboard);
RIVE_RS_API void rive_rs_artboard_set_volume(rive_rs_artboard* artboard, float volume);
RIVE_RS_API rive_rs_status rive_rs_artboard_reset_size(rive_rs_artboard* artboard);
RIVE_RS_API size_t rive_rs_artboard_animation_count(const rive_rs_artboard* artboard);
RIVE_RS_API size_t rive_rs_artboard_state_machine_count(const rive_rs_artboard* artboard);
RIVE_RS_API size_t rive_rs_artboard_event_count(const rive_rs_artboard* artboard);
RIVE_RS_API rive_rs_status rive_rs_artboard_event_at(
    const rive_rs_artboard* artboard,
    size_t index,
    rive_rs_event_info* out_event);
RIVE_RS_API rive_rs_status rive_rs_artboard_event_property_at(
    const rive_rs_artboard* artboard,
    size_t event_index,
    size_t property_index,
    rive_rs_event_property_info* out_property);

RIVE_RS_API rive_rs_status rive_rs_artboard_animation_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_linear_animation** out_animation);
RIVE_RS_API rive_rs_status rive_rs_artboard_animation_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_linear_animation** out_animation);
RIVE_RS_API rive_rs_status rive_rs_artboard_state_machine_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_state_machine** out_state_machine);
RIVE_RS_API rive_rs_status rive_rs_artboard_state_machine_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_state_machine** out_state_machine);
RIVE_RS_API rive_rs_status rive_rs_artboard_input_by_path(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_str_view path,
    rive_rs_smi_input** out_input);
RIVE_RS_API size_t rive_rs_artboard_text_value_run_count(const rive_rs_artboard* artboard);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_value_run_name_at(
    const rive_rs_artboard* artboard,
    size_t index,
    rive_rs_str_view* out_name);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_value_run_text_at(
    const rive_rs_artboard* artboard,
    size_t index,
    rive_rs_str_view* out_text);
RIVE_RS_API rive_rs_status rive_rs_artboard_set_text_value_run_text_at(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_str_view text);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_by_path_get(
    const rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_str_view path,
    rive_rs_str_view* out_text);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_by_path_set(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_str_view path,
    rive_rs_str_view text);
RIVE_RS_API rive_rs_status rive_rs_artboard_transform_component_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_transform_component** out_component);
RIVE_RS_API rive_rs_status rive_rs_artboard_node_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_node** out_node);
RIVE_RS_API rive_rs_status rive_rs_artboard_bone_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_bone** out_bone);
RIVE_RS_API rive_rs_status rive_rs_artboard_root_bone_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_root_bone** out_root_bone);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_value_run_by_name(
    rive_rs_artboard* artboard,
    rive_rs_str_view name,
    rive_rs_text_value_run** out_text_value_run);
RIVE_RS_API rive_rs_status rive_rs_artboard_text_value_run_by_index(
    rive_rs_artboard* artboard,
    size_t index,
    rive_rs_text_value_run** out_text_value_run);
RIVE_RS_API rive_rs_status rive_rs_artboard_flatten_path(
    rive_rs_artboard* artboard,
    size_t index,
    bool transform_to_parent,
    rive_rs_flattened_path** out_path);
RIVE_RS_API rive_rs_status rive_rs_artboard_bind_view_model_instance(
    rive_rs_artboard* artboard,
    rive_rs_view_model_instance* instance);

RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_new(
    int32_t width,
    int32_t height,
    rive_rs_webgl2_renderer** out_renderer);
RIVE_RS_API void rive_rs_webgl2_renderer_delete(
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_clear(
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_flush(
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_resize(
    rive_rs_webgl2_renderer* renderer,
    int32_t width,
    int32_t height);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_save(
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_restore(
    rive_rs_webgl2_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_transform(
    rive_rs_webgl2_renderer* renderer,
    const rive_rs_mat2d* matrix);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_modulate_opacity(
    rive_rs_webgl2_renderer* renderer,
    float opacity);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_align(
    rive_rs_webgl2_renderer* renderer,
    rive_rs_fit fit,
    rive_rs_alignment alignment,
    const rive_rs_aabb* frame,
    const rive_rs_aabb* content,
    float scale_factor);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_save_clip_rect(
    rive_rs_webgl2_renderer* renderer,
    float left,
    float top,
    float right,
    float bottom);
RIVE_RS_API rive_rs_status rive_rs_webgl2_renderer_restore_clip_rect(
    rive_rs_webgl2_renderer* renderer);

RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_new(
    int32_t width,
    int32_t height,
    rive_rs_webgpu_renderer** out_renderer);
RIVE_RS_API void rive_rs_webgpu_renderer_delete(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_clear(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_flush(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_resize(
    rive_rs_webgpu_renderer* renderer,
    int32_t width,
    int32_t height);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_save(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_restore(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_transform(
    rive_rs_webgpu_renderer* renderer,
    const rive_rs_mat2d* matrix);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_modulate_opacity(
    rive_rs_webgpu_renderer* renderer,
    float opacity);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_align(
    rive_rs_webgpu_renderer* renderer,
    rive_rs_fit fit,
    rive_rs_alignment alignment,
    const rive_rs_aabb* frame,
    const rive_rs_aabb* content,
    float scale_factor);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_save_clip_rect(
    rive_rs_webgpu_renderer* renderer,
    float left,
    float top,
    float right,
    float bottom);
RIVE_RS_API rive_rs_status rive_rs_webgpu_renderer_restore_clip_rect(
    rive_rs_webgpu_renderer* renderer);
RIVE_RS_API void rive_rs_bindable_artboard_ref(rive_rs_bindable_artboard* bindable_artboard);
RIVE_RS_API void rive_rs_bindable_artboard_unref(rive_rs_bindable_artboard* bindable_artboard);

RIVE_RS_API float rive_rs_transform_component_scale_x(
    const rive_rs_transform_component* component);
RIVE_RS_API void rive_rs_transform_component_set_scale_x(
    rive_rs_transform_component* component,
    float value);
RIVE_RS_API float rive_rs_transform_component_scale_y(
    const rive_rs_transform_component* component);
RIVE_RS_API void rive_rs_transform_component_set_scale_y(
    rive_rs_transform_component* component,
    float value);
RIVE_RS_API float rive_rs_transform_component_rotation(
    const rive_rs_transform_component* component);
RIVE_RS_API void rive_rs_transform_component_set_rotation(
    rive_rs_transform_component* component,
    float value);
RIVE_RS_API rive_rs_status rive_rs_transform_component_world_transform(
    const rive_rs_transform_component* component,
    rive_rs_mat2d* out_matrix);
RIVE_RS_API rive_rs_status rive_rs_transform_component_parent_world_transform(
    const rive_rs_transform_component* component,
    rive_rs_mat2d* out_matrix);

RIVE_RS_API float rive_rs_node_x(const rive_rs_node* node);
RIVE_RS_API void rive_rs_node_set_x(rive_rs_node* node, float value);
RIVE_RS_API float rive_rs_node_y(const rive_rs_node* node);
RIVE_RS_API void rive_rs_node_set_y(rive_rs_node* node, float value);

RIVE_RS_API float rive_rs_bone_length(const rive_rs_bone* bone);
RIVE_RS_API void rive_rs_bone_set_length(rive_rs_bone* bone, float value);

RIVE_RS_API float rive_rs_root_bone_x(const rive_rs_root_bone* root_bone);
RIVE_RS_API void rive_rs_root_bone_set_x(rive_rs_root_bone* root_bone, float value);
RIVE_RS_API float rive_rs_root_bone_y(const rive_rs_root_bone* root_bone);
RIVE_RS_API void rive_rs_root_bone_set_y(rive_rs_root_bone* root_bone, float value);

RIVE_RS_API rive_rs_str_view rive_rs_text_value_run_name(
    const rive_rs_text_value_run* text_value_run);
RIVE_RS_API rive_rs_str_view rive_rs_text_value_run_text(
    const rive_rs_text_value_run* text_value_run);
RIVE_RS_API rive_rs_status rive_rs_text_value_run_set_text(
    rive_rs_text_value_run* text_value_run,
    rive_rs_str_view text);

RIVE_RS_API void rive_rs_flattened_path_delete(rive_rs_flattened_path* path);
RIVE_RS_API size_t rive_rs_flattened_path_length(const rive_rs_flattened_path* path);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_is_cubic(
    const rive_rs_flattened_path* path,
    size_t index,
    bool* out_is_cubic);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_x(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_y(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_in_x(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_in_y(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_out_x(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_flattened_path_out_y(
    const rive_rs_flattened_path* path,
    size_t index,
    float* out_value);

RIVE_RS_API rive_rs_status rive_rs_linear_animation_instance_new(
    rive_rs_linear_animation* animation,
    rive_rs_artboard* artboard,
    rive_rs_linear_animation_instance** out_instance);
RIVE_RS_API rive_rs_str_view rive_rs_linear_animation_name(
    const rive_rs_linear_animation* animation);
RIVE_RS_API uint32_t rive_rs_linear_animation_duration(
    const rive_rs_linear_animation* animation);
RIVE_RS_API uint32_t rive_rs_linear_animation_fps(const rive_rs_linear_animation* animation);
RIVE_RS_API uint32_t rive_rs_linear_animation_work_start(
    const rive_rs_linear_animation* animation);
RIVE_RS_API uint32_t rive_rs_linear_animation_work_end(
    const rive_rs_linear_animation* animation);
RIVE_RS_API bool rive_rs_linear_animation_enable_work_area(
    const rive_rs_linear_animation* animation);
RIVE_RS_API uint32_t rive_rs_linear_animation_loop_value(
    const rive_rs_linear_animation* animation);
RIVE_RS_API float rive_rs_linear_animation_speed(const rive_rs_linear_animation* animation);
RIVE_RS_API rive_rs_status rive_rs_linear_animation_apply(
    const rive_rs_linear_animation* animation,
    rive_rs_artboard* artboard,
    float time,
    float mix);
RIVE_RS_API void rive_rs_linear_animation_instance_delete(
    rive_rs_linear_animation_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_linear_animation_instance_advance(
    rive_rs_linear_animation_instance* instance,
    float seconds,
    bool* out_looped);
RIVE_RS_API rive_rs_status rive_rs_linear_animation_instance_apply(
    rive_rs_linear_animation_instance* instance,
    rive_rs_artboard* artboard,
    float mix);
RIVE_RS_API float rive_rs_linear_animation_instance_time(
    const rive_rs_linear_animation_instance* instance);
RIVE_RS_API void rive_rs_linear_animation_instance_set_time(
    rive_rs_linear_animation_instance* instance,
    float seconds);
RIVE_RS_API bool rive_rs_linear_animation_instance_did_loop(
    const rive_rs_linear_animation_instance* instance);

RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_new(
    rive_rs_state_machine* state_machine,
    rive_rs_artboard* artboard,
    rive_rs_state_machine_instance** out_instance);
RIVE_RS_API rive_rs_str_view rive_rs_state_machine_name(
    const rive_rs_state_machine* state_machine);
RIVE_RS_API void rive_rs_state_machine_instance_delete(
    rive_rs_state_machine_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_advance(
    rive_rs_state_machine_instance* instance,
    float seconds,
    bool* out_changed);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_advance_and_apply(
    rive_rs_state_machine_instance* instance,
    float seconds,
    bool* out_changed);
RIVE_RS_API size_t rive_rs_state_machine_input_count(
    const rive_rs_state_machine_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_state_machine_input_at(
    rive_rs_state_machine_instance* instance,
    size_t index,
    rive_rs_smi_input** out_input);
RIVE_RS_API rive_rs_smi_input_type rive_rs_smi_input_type_of(const rive_rs_smi_input* input);
RIVE_RS_API rive_rs_str_view rive_rs_smi_input_name(const rive_rs_smi_input* input);
RIVE_RS_API rive_rs_status rive_rs_smi_input_as_bool(
    rive_rs_smi_input* input,
    rive_rs_smi_bool** out_bool);
RIVE_RS_API rive_rs_status rive_rs_smi_input_as_number(
    rive_rs_smi_input* input,
    rive_rs_smi_number** out_number);
RIVE_RS_API rive_rs_status rive_rs_smi_input_as_trigger(
    rive_rs_smi_input* input,
    rive_rs_smi_trigger** out_trigger);
RIVE_RS_API bool rive_rs_smi_bool_get(const rive_rs_smi_bool* input);
RIVE_RS_API void rive_rs_smi_bool_set(rive_rs_smi_bool* input, bool value);
RIVE_RS_API float rive_rs_smi_number_get(const rive_rs_smi_number* input);
RIVE_RS_API void rive_rs_smi_number_set(rive_rs_smi_number* input, float value);
RIVE_RS_API void rive_rs_smi_trigger_fire(rive_rs_smi_trigger* input);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_pointer_down(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_pointer_move(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_pointer_up(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_pointer_exit(
    rive_rs_state_machine_instance* instance,
    rive_rs_vec2 point,
    int32_t pointer_id);
RIVE_RS_API bool rive_rs_state_machine_instance_has_listeners(
    const rive_rs_state_machine_instance* instance);
RIVE_RS_API bool rive_rs_state_machine_instance_has_any_listener(
    const rive_rs_state_machine_instance* instance);
RIVE_RS_API size_t rive_rs_state_machine_reported_event_count(
    const rive_rs_state_machine_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_state_machine_reported_event_at(
    const rive_rs_state_machine_instance* instance,
    size_t index,
    rive_rs_event_info* out_event,
    float* out_delay_seconds);
RIVE_RS_API rive_rs_status rive_rs_state_machine_reported_event_property_at(
    const rive_rs_state_machine_instance* instance,
    size_t reported_event_index,
    size_t property_index,
    rive_rs_event_property_info* out_property);
RIVE_RS_API size_t rive_rs_state_machine_state_changed_count(
    const rive_rs_state_machine_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_state_machine_state_changed_name_at(
    const rive_rs_state_machine_instance* instance,
    size_t index,
    rive_rs_str_view* out_name);
RIVE_RS_API rive_rs_status rive_rs_state_machine_instance_bind_view_model_instance(
    rive_rs_state_machine_instance* instance,
    rive_rs_view_model_instance* view_model_instance);

RIVE_RS_API void rive_rs_view_model_ref(rive_rs_view_model* view_model);
RIVE_RS_API void rive_rs_view_model_unref(rive_rs_view_model* view_model);
RIVE_RS_API rive_rs_str_view rive_rs_view_model_name(const rive_rs_view_model* view_model);
RIVE_RS_API size_t rive_rs_view_model_property_count(const rive_rs_view_model* view_model);
RIVE_RS_API size_t rive_rs_view_model_instance_count(const rive_rs_view_model* view_model);
RIVE_RS_API rive_rs_status rive_rs_view_model_property_at(
    const rive_rs_view_model* view_model,
    size_t index,
    rive_rs_property_info* out_property);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_name_at(
    const rive_rs_view_model* view_model,
    size_t index,
    rive_rs_str_view* out_name);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_by_index(
    const rive_rs_view_model* view_model,
    size_t index,
    rive_rs_view_model_instance** out_instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_by_name(
    const rive_rs_view_model* view_model,
    rive_rs_str_view name,
    rive_rs_view_model_instance** out_instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_default_instance(
    const rive_rs_view_model* view_model,
    rive_rs_view_model_instance** out_instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_new_instance(
    const rive_rs_view_model* view_model,
    rive_rs_view_model_instance** out_instance);

RIVE_RS_API void rive_rs_view_model_instance_ref(rive_rs_view_model_instance* instance);
RIVE_RS_API void rive_rs_view_model_instance_unref(rive_rs_view_model_instance* instance);
RIVE_RS_API size_t rive_rs_view_model_instance_property_count(
    const rive_rs_view_model_instance* instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_property_at(
    const rive_rs_view_model_instance* instance,
    size_t index,
    rive_rs_property_info* out_property);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_number(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    float* out_value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_number(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    float value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_string(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_str_view* out_value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_string(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_str_view value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_boolean(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    bool* out_value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_boolean(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    bool value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_color(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    int32_t* out_argb);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_color(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    int32_t argb);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_enum(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_str_view* out_value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_enum(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_str_view value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_enum_index(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    uint32_t* out_index);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_enum_index(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    uint32_t index);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_fire_trigger(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_view_model(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance** out_instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_replace_view_model(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_property_has_changed(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    bool* out_changed);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_clear_property_changes(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_size(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t* out_size);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_instance_at(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t index,
    rive_rs_view_model_instance** out_value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_add_instance(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_add_instance_at(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value,
    size_t index,
    bool* out_added);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_remove_instance(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_remove_instance_at(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    size_t index);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_list_swap(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    uint32_t a,
    uint32_t b);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_artboard(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_bindable_artboard* value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_artboard_view_model(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_view_model_instance* view_model_instance);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_set_image(
    rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_render_image* value);
RIVE_RS_API rive_rs_status rive_rs_view_model_instance_get_image(
    const rive_rs_view_model_instance* instance,
    rive_rs_str_view path,
    rive_rs_render_image** out_value);

RIVE_RS_API rive_rs_status rive_rs_compute_alignment(
    rive_rs_fit fit,
    rive_rs_alignment alignment,
    const rive_rs_aabb* source,
    const rive_rs_aabb* destination,
    float scale_factor,
    rive_rs_mat2d* out_matrix);
RIVE_RS_API rive_rs_status rive_rs_map_xy(
    const rive_rs_mat2d* matrix,
    rive_rs_vec2 point,
    rive_rs_vec2* out_point);

RIVE_RS_API rive_rs_status rive_rs_decode_audio(
    rive_rs_factory* factory,
    rive_rs_bytes_view bytes,
    rive_rs_audio_source** out_audio);
RIVE_RS_API rive_rs_status rive_rs_decode_font(
    rive_rs_factory* factory,
    rive_rs_bytes_view bytes,
    rive_rs_font** out_font);
RIVE_RS_API rive_rs_status rive_rs_decode_webgl2_image(
    rive_rs_bytes_view bytes,
    rive_rs_render_image** out_image);
RIVE_RS_API void rive_rs_audio_source_unref(rive_rs_audio_source* audio);
RIVE_RS_API void rive_rs_font_unref(rive_rs_font* font);
RIVE_RS_API void rive_rs_render_image_ref(rive_rs_render_image* image);
RIVE_RS_API void rive_rs_render_image_unref(rive_rs_render_image* image);

RIVE_RS_API rive_rs_file_asset* rive_rs_ptr_to_file_asset(uintptr_t pointer);
RIVE_RS_API rive_rs_file_asset* rive_rs_ptr_to_audio_asset(uintptr_t pointer);
RIVE_RS_API rive_rs_file_asset* rive_rs_ptr_to_image_asset(uintptr_t pointer);
RIVE_RS_API rive_rs_file_asset* rive_rs_ptr_to_font_asset(uintptr_t pointer);
RIVE_RS_API rive_rs_str_view rive_rs_file_asset_name(const rive_rs_file_asset* asset);
RIVE_RS_API rive_rs_str_view rive_rs_file_asset_cdn_base_url(
    const rive_rs_file_asset* asset);
RIVE_RS_API rive_rs_str_view rive_rs_file_asset_file_extension(
    const rive_rs_file_asset* asset);
RIVE_RS_API rive_rs_str_view rive_rs_file_asset_unique_filename(
    const rive_rs_file_asset* asset);
RIVE_RS_API bool rive_rs_file_asset_is_audio(const rive_rs_file_asset* asset);
RIVE_RS_API bool rive_rs_file_asset_is_image(const rive_rs_file_asset* asset);
RIVE_RS_API bool rive_rs_file_asset_is_font(const rive_rs_file_asset* asset);
RIVE_RS_API rive_rs_str_view rive_rs_file_asset_cdn_uuid(
    const rive_rs_file_asset* asset);

RIVE_RS_API rive_rs_status rive_rs_file_asset_decode(
    rive_rs_factory* factory,
    rive_rs_file_asset* asset,
    rive_rs_bytes_view bytes);
RIVE_RS_API rive_rs_status rive_rs_audio_asset_set_audio_source(
    rive_rs_file_asset* asset,
    rive_rs_audio_source* audio);
RIVE_RS_API rive_rs_status rive_rs_font_asset_set_font(
    rive_rs_file_asset* asset,
    rive_rs_font* font);
RIVE_RS_API rive_rs_status rive_rs_image_asset_set_render_image(
    rive_rs_file_asset* asset,
    rive_rs_render_image* image);

#ifdef __cplusplus
}
#endif

#endif
