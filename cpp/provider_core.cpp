#include "rive_rs_abi.h"

#include "rive/factory.hpp"
#include "rive/refcnt.hpp"

#include <atomic>
#include <cstdint>
#include <new>

namespace
{
class NoOpRenderImage : public rive::RenderImage
{
public:
    NoOpRenderImage() = default;
};

class NoOpRenderPaint : public rive::RenderPaint
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

class NoOpRenderPath : public rive::RenderPath
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

class NoOpRenderShader : public rive::RenderShader
{
public:
    NoOpRenderShader() = default;
};

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
} // namespace

struct rive_rs_factory
{
    std::atomic_uint32_t refs;
    rive::Factory* factory;
};

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
    handle->refs.store(1, std::memory_order_relaxed);
    handle->factory = new (std::nothrow) NoOpFactory();
    if (handle->factory == nullptr)
    {
        delete handle;
        return nullptr;
    }
    return handle;
}

void rive_rs_factory_ref(rive_rs_factory* factory)
{
    if (factory == nullptr)
    {
        return;
    }
    (void)factory->refs.fetch_add(1, std::memory_order_relaxed);
}

void rive_rs_factory_unref(rive_rs_factory* factory)
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

rive_rs_status rive_rs_map_xy(const rive_rs_mat2d* matrix,
                              rive_rs_vec2 point,
                              rive_rs_vec2* out_point)
{
    if (matrix == nullptr || out_point == nullptr)
    {
        return RIVE_RS_STATUS_NULL;
    }
    out_point->x = matrix->xx * point.x + matrix->yx * point.y + matrix->tx;
    out_point->y = matrix->xy * point.x + matrix->yy * point.y + matrix->ty;
    return RIVE_RS_STATUS_OK;
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
} // extern "C"
