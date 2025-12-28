#include <hyprgraphics/image/Image.hpp>
#include "utils/Format.hpp"
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

Hyprgraphics::CImage::CImage(std::span<const uint8_t> data, eImageFormat format, const Vector2D& size) {

    const auto FORMAT = format == IMAGE_FORMAT_AUTO ? formatFromStream(data) : format;

    if (FORMAT == IMAGE_FORMAT_ERROR) {
        lastError = "invalid file";
        return;
    }

    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;

    switch (FORMAT) {
        case IMAGE_FORMAT_PNG: CAIROSURFACE = PNG::createSurfaceFromPNG(data); break;
#ifdef HEIF_FOUND
        case IMAGE_FORMAT_AVIF: CAIROSURFACE = AVIF::createSurfaceFromAvif(data); break;
#else
        case IMAGE_FORMAT_AVIF: lastError = "hyprgraphics compiled without HEIF support"; return;
#endif
        default: lastError = "Currently only PNG and AVIF images are supported for embedding"; return;
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

    const auto FORMAT = formatFromFile(path);

    if (FORMAT == IMAGE_FORMAT_ERROR) {
        lastError = "invalid file";
        return;
    }

    std::expected<cairo_surface_t*, std::string> CAIROSURFACE;

    mime = mimeOf(FORMAT);

    switch (FORMAT) {
        case IMAGE_FORMAT_PNG: CAIROSURFACE = PNG::createSurfaceFromPNG(path); break;
        case IMAGE_FORMAT_BMP: CAIROSURFACE = BMP::createSurfaceFromBMP(path); break;
#ifdef HEIF_FOUND
        case IMAGE_FORMAT_AVIF: CAIROSURFACE = AVIF::createSurfaceFromAvif(path); break;
#else
        case IMAGE_FORMAT_AVIF: lastError = "hyprgraphics compiled without HEIF support"; return;
#endif
#ifdef JXL_FOUND
        case IMAGE_FORMAT_JXL: CAIROSURFACE = JXL::createSurfaceFromJXL(path); break;
#else
        case IMAGE_FORMAT_JXL: lastError = "hyprgraphics compiled without JXL support"; return;
#endif
        case IMAGE_FORMAT_JPEG: CAIROSURFACE = JPEG::createSurfaceFromJPEG(path); break;
        case IMAGE_FORMAT_SVG: CAIROSURFACE = SVG::createSurfaceFromSVG(path, size); break;
        case IMAGE_FORMAT_WEBP: CAIROSURFACE = WEBP::createSurfaceFromWEBP(path); break;
        default: lastError = "internal error"; return;
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
