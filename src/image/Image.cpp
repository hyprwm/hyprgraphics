#include <hyprgraphics/image/Image.hpp>
#include "formats/Bmp.hpp"
#include "formats/Jpeg.hpp"
#ifdef JXL_FOUND
#include "formats/JpegXL.hpp"
#endif
#ifdef HEIF_FOUND
#include "formats/Avif.hpp"
#endif
#include "formats/Webp.hpp"
#include "formats/Png.hpp"
#include "formats/Svg.hpp"
#include <magic.h>
#include <format>

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;
using namespace Hyprutils::Math;

Hyprgraphics::CImage::CImage(const std::span<uint8_t>& data, eImageFormat format) {
    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;
    if (format == eImageFormat::IMAGE_FORMAT_PNG) {
        CAIROSURFACE = PNG::createSurfaceFromPNG(data);
        mime         = "image/png";
    } else if (format == eImageFormat::IMAGE_FORMAT_AVIF) {
#ifndef HEIF_FOUND
        lastError = "hyprgraphics compiled without HEIF support";
        return;
#else
        CAIROSURFACE = AVIF::createSurfaceFromAvif(data);
        mime         = "image/avif";
#endif
    } else {
        lastError = "Currently only PNG and AVIF images are supported for embedding";
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

Hyprgraphics::CImage::CImage(const std::string& path, const Vector2D& size) : filepath(path), m_svgSize(size) {
    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;
    if (path.ends_with(".png") || path.ends_with(".PNG")) {
        CAIROSURFACE = PNG::createSurfaceFromPNG(path);
        mime         = "image/png";
    } else if (path.ends_with(".jpg") || path.ends_with(".JPG") || path.ends_with(".jpeg") || path.ends_with(".JPEG")) {
        CAIROSURFACE  = JPEG::createSurfaceFromJPEG(path);
        imageHasAlpha = false;
        mime          = "image/jpeg";
    } else if (path.ends_with(".bmp") || path.ends_with(".BMP")) {
        CAIROSURFACE  = BMP::createSurfaceFromBMP(path);
        imageHasAlpha = false;
        mime          = "image/bmp";
    } else if (path.ends_with(".webp") || path.ends_with(".WEBP")) {
        CAIROSURFACE = WEBP::createSurfaceFromWEBP(path);
        mime         = "image/webp";
    } else if (path.ends_with(".svg") || path.ends_with(".SVG")) {
        CAIROSURFACE = SVG::createSurfaceFromSVG(path, m_svgSize);
        mime         = "image/svg";
    } else if (path.ends_with(".jxl") || path.ends_with(".JXL")) {

#ifdef JXL_FOUND
        CAIROSURFACE = JXL::createSurfaceFromJXL(path);
        mime         = "image/jxl";
#else
        lastError = "hyprgraphics compiled without JXL support";
        return;
#endif

    } else if (path.ends_with(".avif") || path.ends_with(".AVIF")) {

#ifdef HEIF_FOUND
        CAIROSURFACE = AVIF::createSurfaceFromAvif(path);
        mime         = "image/avif";
#else
        lastError = "hyprgraphics compiled without HEIF support";
        return;
#endif

    } else {
        // magic is slow, so only use it when no recognized extension is found
        auto handle = magic_open(MAGIC_NONE | MAGIC_COMPRESS | MAGIC_SYMLINK);
        magic_load(handle, nullptr);

        const auto type_str = std::string(magic_file(handle, path.c_str()));

        magic_close(handle);

        const auto first_word = type_str.substr(0, type_str.find(' '));

        if (first_word == "PNG") {
            CAIROSURFACE = PNG::createSurfaceFromPNG(path);
            mime         = "image/png";
        } else if (first_word == "JPEG" && !type_str.contains("XL") && !type_str.contains("2000")) {
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
        } else if (type_str.contains("AVIF")) { // libmagic can identify AVIF images as "ISO Media, AVIF Image"
#ifdef HEIF_FOUND
            CAIROSURFACE = AVIF::createSurfaceFromAvif(path);
            mime         = "image/avif";
#else
            lastError = "hyprgraphics compiled without AVIF support";
            return;
#endif
        } else if (first_word == "BMP") {
            CAIROSURFACE  = BMP::createSurfaceFromBMP(path);
            imageHasAlpha = false;
            mime          = "image/bmp";
        } else if (first_word == "SVG") {
            CAIROSURFACE  = SVG::createSurfaceFromSVG(path, m_svgSize);
            imageHasAlpha = false;
            mime          = "image/svg";
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
