#include <cmath>
#include <hyprgraphics/egl/Egl.hpp>
#include <hyprutils/memory/Casts.hpp>
#include <vector>
#include <GLES2/gl2ext.h>
#include <GLES3/gl32.h>
#include <xf86drm.h>
#include <drm_fourcc.h>

using namespace Hyprutils::Memory;

namespace Hyprgraphics::Egl {
    static inline const std::vector<SPixelFormat> GLES3_FORMATS = {
        {
            .drmFormat        = DRM_FORMAT_ARGB8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XRGB8888,
            .alphaAdded       = DRM_FORMAT_ARGB8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_BGRA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {8, 8, 8, 8},
        },
        {
            .drmFormat        = DRM_FORMAT_XRGB8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XRGB8888,
            .alphaAdded       = DRM_FORMAT_ARGB8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_BGR1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {8, 8, 8, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_XBGR8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XBGR8888,
            .alphaAdded       = DRM_FORMAT_ABGR8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {8, 8, 8, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ABGR8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XBGR8888,
            .alphaAdded       = DRM_FORMAT_ABGR8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {8, 8, 8, 8},
        },
        {
            .drmFormat        = DRM_FORMAT_BGR888,
            .glInternalFormat = GL_RGB8,
            .glFormat         = GL_RGB,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_BGR888,
            .alphaAdded       = 0,
            .bytesPerBlock    = 3,
            .swizzle          = {SWIZZLE_BGR1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGR,
            .bits             = {8, 8, 8, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBX4444,
            .glInternalFormat = GL_RGBA4,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_SHORT_4_4_4_4,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_RGBX4444,
            .alphaAdded       = DRM_FORMAT_RGBA4444,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {4, 4, 4, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBA4444,
            .glInternalFormat = GL_RGBA4,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_SHORT_4_4_4_4,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_RGBX4444,
            .alphaAdded       = DRM_FORMAT_RGBA4444,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {4, 4, 4, 4},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBX5551,
            .glInternalFormat = GL_RGB5_A1,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_SHORT_5_5_5_1,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_RGBX5551,
            .alphaAdded       = DRM_FORMAT_RGBA5551,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {5, 5, 5, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBA5551,
            .glInternalFormat = GL_RGB5_A1,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_SHORT_5_5_5_1,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_RGBX5551,
            .alphaAdded       = DRM_FORMAT_RGBA5551,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {5, 5, 5, 1},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBX8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_RGBX8888,
            .alphaAdded       = DRM_FORMAT_RGBA8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_ABG1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {8, 8, 8, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_RGBA8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_RGBX8888,
            .alphaAdded       = DRM_FORMAT_RGBA8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_ABGR},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {8, 8, 8, 8},
        },
        {
            .drmFormat        = DRM_FORMAT_BGRX8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_BGRX8888,
            .alphaAdded       = DRM_FORMAT_BGRA8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_GBA1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {8, 8, 8, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_BGRA8888,
            .glInternalFormat = GL_RGBA8,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_BGRX8888,
            .alphaAdded       = DRM_FORMAT_BGRA8888,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_GBAR},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {8, 8, 8, 8},
        },
        {
            .drmFormat        = DRM_FORMAT_RGB565,
            .glInternalFormat = GL_RGB565,
            .glFormat         = GL_RGB,
            .glType           = GL_UNSIGNED_SHORT_5_6_5,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_RGB565,
            .alphaAdded       = 0,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGB,
            .bits             = {5, 6, 5, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_XBGR2101010,
            .glInternalFormat = GL_RGB10_A2,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_INT_2_10_10_10_REV,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XBGR2101010,
            .alphaAdded       = DRM_FORMAT_ABGR2101010,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {10, 10, 10, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ABGR2101010,
            .glInternalFormat = GL_RGB10_A2,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_INT_2_10_10_10_REV,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XBGR2101010,
            .alphaAdded       = DRM_FORMAT_ABGR2101010,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {10, 10, 10, 2},
        },
        {
            .drmFormat        = DRM_FORMAT_XRGB2101010,
            .glInternalFormat = GL_RGB10_A2,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_INT_2_10_10_10_REV,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XRGB2101010,
            .alphaAdded       = DRM_FORMAT_ARGB2101010,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_BGR1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {10, 10, 10, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ARGB2101010,
            .glInternalFormat = GL_RGB10_A2,
            .glFormat         = GL_RGBA,
            .glType           = GL_UNSIGNED_INT_2_10_10_10_REV,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XRGB2101010,
            .alphaAdded       = DRM_FORMAT_ARGB2101010,
            .bytesPerBlock    = 4,
            .swizzle          = {SWIZZLE_BGRA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {10, 10, 10, 2},
        },
        {
            .drmFormat        = DRM_FORMAT_XRGB16161616F,
            .glInternalFormat = GL_RGBA16F,
            .glFormat         = GL_RGBA,
            .glType           = GL_HALF_FLOAT,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XRGB16161616F,
            .alphaAdded       = DRM_FORMAT_ARGB16161616F,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_BGR1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {16, 16, 16, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ARGB16161616F,
            .glInternalFormat = GL_RGBA16F,
            .glFormat         = GL_RGBA,
            .glType           = GL_HALF_FLOAT,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XRGB16161616F,
            .alphaAdded       = DRM_FORMAT_ARGB16161616F,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_BGRA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_BGRA,
            .bits             = {16, 16, 16, 16},
        },
        {
            .drmFormat        = DRM_FORMAT_XBGR16161616F,
            .glInternalFormat = GL_RGBA16F,
            .glFormat         = GL_RGBA,
            .glType           = GL_HALF_FLOAT,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XBGR16161616F,
            .alphaAdded       = DRM_FORMAT_ABGR16161616F,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {16, 16, 16, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ABGR16161616F,
            .glInternalFormat = GL_RGBA16F,
            .glFormat         = GL_RGBA,
            .glType           = GL_HALF_FLOAT,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XBGR16161616F,
            .alphaAdded       = DRM_FORMAT_ABGR16161616F,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA,
            .bits             = {16, 16, 16, 16},
        },
        {
            .drmFormat        = DRM_FORMAT_XBGR16161616,
            .glInternalFormat = GL_RGBA16UI,
            .glFormat         = GL_RGBA_INTEGER,
            .glType           = GL_UNSIGNED_SHORT,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_XBGR16161616,
            .alphaAdded       = DRM_FORMAT_ABGR16161616,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA_I,
            .bits             = {16, 16, 16, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_ABGR16161616,
            .glInternalFormat = GL_RGBA16UI,
            .glFormat         = GL_RGBA_INTEGER,
            .glType           = GL_UNSIGNED_SHORT,
            .withAlpha        = true,
            .alphaStripped    = DRM_FORMAT_XBGR16161616,
            .alphaAdded       = DRM_FORMAT_ABGR16161616,
            .bytesPerBlock    = 8,
            .swizzle          = {SWIZZLE_RGBA},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGBA_I,
            .bits             = {16, 16, 16, 16},
        },
        {
            .drmFormat     = DRM_FORMAT_YVYU,
            .bytesPerBlock = 4,
            .blockSize     = {2, 1},
        },
        {
            .drmFormat     = DRM_FORMAT_VYUY,
            .bytesPerBlock = 4,
            .blockSize     = {2, 1},
        },
        {
            .drmFormat        = DRM_FORMAT_R8,
            .glInternalFormat = GL_R8,
            .glFormat         = GL_RED,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_R8,
            .alphaAdded       = 0,
            .bytesPerBlock    = 1,
            .swizzle          = {SWIZZLE_R001},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_R,
            .bits             = {8, 0, 0, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_GR88,
            .glInternalFormat = GL_RG8,
            .glFormat         = GL_RG,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_GR88,
            .alphaAdded       = 0,
            .bytesPerBlock    = 2,
            .swizzle          = {SWIZZLE_RG01},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RG,
            .bits             = {8, 8, 0, 0},
        },
        {
            .drmFormat        = DRM_FORMAT_RGB888,
            .glInternalFormat = GL_RGB8,
            .glFormat         = GL_RGB,
            .glType           = GL_UNSIGNED_BYTE,
            .withAlpha        = false,
            .alphaStripped    = DRM_FORMAT_RGB888,
            .alphaAdded       = 0,
            .bytesPerBlock    = 3,
            .swizzle          = {SWIZZLE_RGB1},
            .pixelLayout      = EPixelLayout::PIXEL_LAYOUT_RGB,
            .bits             = {8, 8, 8, 0},
        },
    };

    const SPixelFormat* getPixelFormatFromDRM(uint32_t drmFormat) {
        for (auto const& fmt : GLES3_FORMATS) {
            if (fmt.drmFormat == drmFormat)
                return &fmt;
        }

        return nullptr;
    }

    const SPixelFormat* getPixelFormatFromGL(uint32_t glFormat, uint32_t glType, bool alpha) {
        for (auto const& fmt : GLES3_FORMATS) {
            if (fmt.glFormat == sc<int>(glFormat) && fmt.glType == sc<int>(glType) && fmt.withAlpha == alpha)
                return &fmt;
        }

        return nullptr;
    }

    bool isDrmFormatOpaque(uint32_t drmFormat) {
        const auto FMT = getPixelFormatFromDRM(drmFormat);
        if (!FMT)
            return false;

        return !FMT->withAlpha;
    }

    int pixelsPerBlock(const SPixelFormat* const fmt) {
        return fmt->blockSize.x * fmt->blockSize.y > 0 ? fmt->blockSize.x * fmt->blockSize.y : 1;
    }

    int minStride(const SPixelFormat* const fmt, int32_t width) {
        return std::ceil((width * fmt->bytesPerBlock) / pixelsPerBlock(fmt));
    }

    GLenum getReadbackFormat(const SPixelFormat& fmt) {
        switch (fmt.pixelLayout) {
            case EPixelLayout::PIXEL_LAYOUT_RGBA_I: return GL_RGBA_INTEGER;

            case EPixelLayout::PIXEL_LAYOUT_RGBA: return GL_RGBA;

            case EPixelLayout::PIXEL_LAYOUT_BGRA: return GL_BGRA_EXT;

            case EPixelLayout::PIXEL_LAYOUT_RGB: return GL_RGB;

            case EPixelLayout::PIXEL_LAYOUT_BGR: return GL_BGR_EXT;

            case EPixelLayout::PIXEL_LAYOUT_RG: return GL_RG;

            case EPixelLayout::PIXEL_LAYOUT_R: return GL_RED;

            default: return GL_RGBA;
        }
    }

    uint8_t getColorDepth(const SPixelFormat& fmt) {
        return std::max({fmt.bits.r, fmt.bits.g, fmt.bits.b});
    }
}
