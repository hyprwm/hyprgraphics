#include <hyprgraphics/image/Image.hpp>
#include "formats/Bmp.hpp"
#include "formats/Jpeg.hpp"
#ifdef JXL_FOUND
#include "formats/JpegXL.hpp"
#endif
#include "formats/Webp.hpp"
#include "formats/Png.hpp"
#include <magic.h>
#include <format>

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

Hyprgraphics::CImage::CImage(const std::span<uint8_t>& data, eImageFormat format) {
    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;
    if (format == eImageFormat::IMAGE_FORMAT_PNG) {
        CAIROSURFACE = PNG::createSurfaceFromPNG(data);
        mime         = "image/png";
    } else {
        lastError = "Currently only PNG images are supported for embedding";
        return;
    }

    if (!CAIROSURFACE) {
        lastError = CAIROSURFACE.error();
        return;
    }

    if (const auto STATUS = cairo_surface_status(*CAIROSURFACE); STATUS != CAIRO_STATUS_SUCCESS) {
        lastError = std::format("Could not create surface: {}", cairo_status_to_string(STATUS));
        return;
    }

    loadSuccess   = true;
    pCairoSurface = makeShared<CCairoSurface>(CAIROSURFACE.value());
}

Hyprgraphics::CImage::CImage(const std::string& path) : filepath(path) {
    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;
    const auto                                   len = path.length();
    if (path.find(".png") == len - 4 || path.find(".PNG") == len - 4) {
        CAIROSURFACE = PNG::createSurfaceFromPNG(path);
        mime         = "image/png";
    } else if (path.find(".jpg") == len - 4 || path.find(".JPG") == len - 4 || path.find(".jpeg") == len - 5 || path.find(".JPEG") == len - 5) {
        CAIROSURFACE  = JPEG::createSurfaceFromJPEG(path);
        imageHasAlpha = false;
        mime          = "image/jpeg";
    } else if (path.find(".bmp") == len - 4 || path.find(".BMP") == len - 4) {
        CAIROSURFACE  = BMP::createSurfaceFromBMP(path);
        imageHasAlpha = false;
        mime          = "image/bmp";
    } else if (path.find(".webp") == len - 5 || path.find(".WEBP") == len - 5) {
        CAIROSURFACE = WEBP::createSurfaceFromWEBP(path);
        mime         = "image/webp";
    } else if (path.find(".jxl") == len - 4 || path.find(".JXL") == len - 4) {

#ifdef JXL_FOUND
        CAIROSURFACE = JXL::createSurfaceFromJXL(path);
        mime         = "image/jxl";
#else
        lastError = "hyprgraphics compiled without JXL support";
        return;
#endif

    } else {
        // magic is slow, so only use it when no recognized extension is found
        auto handle = magic_open(MAGIC_NONE | MAGIC_COMPRESS | MAGIC_SYMLINK);
        magic_load(handle, nullptr);

        const auto type_str   = std::string(magic_file(handle, path.c_str()));
        const auto first_word = type_str.substr(0, type_str.find(' '));

        if (first_word == "PNG") {
            CAIROSURFACE = PNG::createSurfaceFromPNG(path);
            mime         = "image/png";
        } else if (first_word == "JPEG" && !type_str.contains("XL") && !type_str.contains("2000") {
            CAIROSURFACE  = JPEG::createSurfaceFromJPEG(path);
            imageHasAlpha = false;
            mime          = "image/jpeg";
        } else if (first_word == "JPEG" && type_str.contains("XL")) {
#ifdef JXL_FOUND
            CAIROSURFACE = JXL::createSurfaceFromJXL(path);
            mime         = "image/jxl";
#else
            lastError = "hyprgraphics compiled without JXL support";
            return;
#endif
        } else if (first_word == "BMP") {
            CAIROSURFACE  = BMP::createSurfaceFromBMP(path);
            imageHasAlpha = false;
            mime          = "image/bmp";
        } else {
            lastError = "unrecognized image";
            return;
        }
    }

    if (!CAIROSURFACE) {
        lastError = CAIROSURFACE.error();
        return;
    }

    if (const auto STATUS = cairo_surface_status(*CAIROSURFACE); STATUS != CAIRO_STATUS_SUCCESS) {
        lastError = std::format("Could not create surface: {}", cairo_status_to_string(STATUS));
        return;
    }

    loadSuccess   = true;
    pCairoSurface = makeShared<CCairoSurface>(CAIROSURFACE.value());
}

Hyprgraphics::CImage::~CImage() {
    ;
}

bool Hyprgraphics::CImage::success() {
    return loadSuccess;
}

bool Hyprgraphics::CImage::hasAlpha() {
    return imageHasAlpha;
}

std::string Hyprgraphics::CImage::getError() {
    return lastError;
}

Hyprutils::Memory::CSharedPointer<CCairoSurface> Hyprgraphics::CImage::cairoSurface() {
    return pCairoSurface;
}

std::string Hyprgraphics::CImage::getMime() {
    return mime;
}
