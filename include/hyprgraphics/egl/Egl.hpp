#pragma once

#include <array>
#include <cstdint>
#include <GLES3/gl32.h>
#include <optional>
#include <hyprutils/math/Vector2D.hpp>

namespace Hyprgraphics::Egl {
    inline constexpr std::array<GLint, 4> SWIZZLE_A1GB{GL_ALPHA, GL_ONE, GL_GREEN, GL_BLUE};
    inline constexpr std::array<GLint, 4> SWIZZLE_ABG1{GL_ALPHA, GL_BLUE, GL_GREEN, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_ABGR{GL_ALPHA, GL_BLUE, GL_GREEN, GL_RED};
    inline constexpr std::array<GLint, 4> SWIZZLE_ARGB{GL_ALPHA, GL_RED, GL_GREEN, GL_BLUE};
    inline constexpr std::array<GLint, 4> SWIZZLE_B1RG{GL_BLUE, GL_ONE, GL_RED, GL_GREEN};
    inline constexpr std::array<GLint, 4> SWIZZLE_BARG{GL_BLUE, GL_ALPHA, GL_RED, GL_GREEN};
    inline constexpr std::array<GLint, 4> SWIZZLE_BGR1{GL_BLUE, GL_GREEN, GL_RED, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_BGRA{GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
    inline constexpr std::array<GLint, 4> SWIZZLE_G1AB{GL_GREEN, GL_ONE, GL_ALPHA, GL_BLUE};
    inline constexpr std::array<GLint, 4> SWIZZLE_GBA1{GL_GREEN, GL_BLUE, GL_ALPHA, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_GBAR{GL_GREEN, GL_BLUE, GL_ALPHA, GL_RED};
    inline constexpr std::array<GLint, 4> SWIZZLE_GRAB{GL_GREEN, GL_RED, GL_ALPHA, GL_BLUE};
    inline constexpr std::array<GLint, 4> SWIZZLE_R001{GL_RED, GL_ZERO, GL_ZERO, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_R1BG{GL_RED, GL_ONE, GL_BLUE, GL_GREEN};
    inline constexpr std::array<GLint, 4> SWIZZLE_RABG{GL_RED, GL_ALPHA, GL_BLUE, GL_GREEN};
    inline constexpr std::array<GLint, 4> SWIZZLE_RG01{GL_RED, GL_GREEN, GL_ZERO, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_GR01{GL_GREEN, GL_RED, GL_ZERO, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_RGB1{GL_RED, GL_GREEN, GL_BLUE, GL_ONE};
    inline constexpr std::array<GLint, 4> SWIZZLE_RGBA{GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};

    enum class EPixelLayout : uint8_t {
        PIXEL_LAYOUT_R,
        PIXEL_LAYOUT_RG,
        PIXEL_LAYOUT_RGB,
        PIXEL_LAYOUT_BGR,
        PIXEL_LAYOUT_RGBA,
        PIXEL_LAYOUT_RGBA_I,
        PIXEL_LAYOUT_BGRA,
        PIXEL_LAYOUT_UNKNOWN,
    };

    struct SFormatBits {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 0;
    };

    struct SPixelFormat {
        uint32_t                            drmFormat        = 0; /* DRM_FORMAT_INVALID */
        int                                 glInternalFormat = 0;
        int                                 glFormat         = 0;
        int                                 glType           = 0;
        bool                                withAlpha        = true;
        uint32_t                            alphaStripped    = 0; /* DRM_FORMAT_INVALID */
        uint32_t                            alphaAdded       = 0; /* DRM_FORMAT_INVALID */
        uint32_t                            bytesPerBlock    = 0;
        Hyprutils::Math::Vector2D           blockSize;
        std::optional<std::array<GLint, 4>> swizzle     = std::nullopt;
        EPixelLayout                        pixelLayout = EPixelLayout::PIXEL_LAYOUT_UNKNOWN;
        SFormatBits                         bits;
    };

    const SPixelFormat* getPixelFormatFromDRM(uint32_t drmFormat);
    const SPixelFormat* getPixelFormatFromGL(uint32_t glFormat, uint32_t glType, bool alpha);
    bool                isDrmFormatOpaque(uint32_t drmFormat);
    int                 pixelsPerBlock(const SPixelFormat* const fmt);
    int                 minStride(const SPixelFormat* const fmt, int32_t width);
    GLenum              getReadbackFormat(const SPixelFormat& fmt);
    uint8_t             getColorDepth(const SPixelFormat& fmt);
}
