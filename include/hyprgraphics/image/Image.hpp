#pragma once

#include <string>
#include <cairo/cairo.h>
#include "../cairo/CairoSurface.hpp"
#include <hyprutils/memory/SharedPtr.hpp>

namespace Hyprgraphics {
    enum EImageFormat {
        IMAGE_FORMAT_PNG
    };

    class CImage {
      public:
        CImage(const std::string& path);
        CImage(const std::span<uint8_t>&, EImageFormat);
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
        Hyprutils::Memory::CSharedPointer<CCairoSurface> pCairoSurface;
        bool                                             imageHasAlpha = true, loadSuccess = false;
    };
};
