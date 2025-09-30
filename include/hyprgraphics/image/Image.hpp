#pragma once

#include <string>
#include <span>
#include <cairo/cairo.h>
#include "../cairo/CairoSurface.hpp"
#include <hyprutils/memory/SharedPtr.hpp>

namespace Hyprgraphics {
    enum eImageFormat : uint8_t {
        IMAGE_FORMAT_PNG,
        IMAGE_FORMAT_AVIF,
    };

    class CImage {
      public:
        CImage(const std::string& path, const Hyprutils::Math::Vector2D& size = {} /* for SVG */);
        CImage(const std::span<uint8_t>&, eImageFormat);
        ~CImage();

        CImage(const CImage&)            = delete;
        CImage& operator=(const CImage&) = delete;

        //
        bool                                             success();
        bool                                             hasAlpha();
        std::string                                      getError();
        std::string                                      getMime();

        Hyprutils::Memory::CSharedPointer<CCairoSurface> cairoSurface();

      private:
        std::string                                      lastError, filepath, mime;
        Hyprutils::Math::Vector2D                        m_svgSize;
        Hyprutils::Memory::CSharedPointer<CCairoSurface> pCairoSurface;
        bool                                             imageHasAlpha = true, loadSuccess = false;
    };
};
