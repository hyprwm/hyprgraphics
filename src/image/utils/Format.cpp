#include "Format.hpp"

#include <magic.h>

#include <hyprutils/utils/ScopeGuard.hpp>

using namespace Hyprgraphics;
using namespace Hyprutils::Utils;

static eImageFormat formatFromStr(const std::string& r) {
    if (r == "image/png")
        return IMAGE_FORMAT_PNG;
    if (r == "image/jpeg" || r == "image/jpg")
        return IMAGE_FORMAT_JPEG;
    if (r == "image/bmp")
        return IMAGE_FORMAT_BMP;
    if (r == "image/webp")
        return IMAGE_FORMAT_WEBP;
    if (r == "image/svg" || r.starts_with("image/svg") /* +xml */)
        return IMAGE_FORMAT_SVG;
    if (r == "image/jxl")
        return IMAGE_FORMAT_JXL;
    if (r == "image/avif")
        return IMAGE_FORMAT_AVIF;

    return IMAGE_FORMAT_ERROR;
}

static eImageFormat formatOf(const std::span<const uint8_t>& data) {
    magic_t m = magic_open(MAGIC_MIME_TYPE);
    if (!m)
        return IMAGE_FORMAT_ERROR;

    CScopeGuard x([&] {
        magic_close(m); //
    });

    if (magic_load(m, nullptr) != 0)
        return IMAGE_FORMAT_ERROR;

    const char* result = magic_buffer(m, data.data(), data.size());
    if (!result)
        return IMAGE_FORMAT_ERROR;

    auto r = std::string{result};
    return formatFromStr(r);
}

static eImageFormat formatOf(const std::string& path) {
    magic_t m = magic_open(MAGIC_MIME_TYPE | MAGIC_SYMLINK);
    if (!m)
        return IMAGE_FORMAT_ERROR;

    CScopeGuard x([&] {
        magic_close(m); //
    });

    if (magic_load(m, nullptr) != 0)
        return IMAGE_FORMAT_ERROR;

    const char* result = magic_file(m, path.c_str());
    if (!result)
        return IMAGE_FORMAT_ERROR;

    auto r = std::string{result};
    return formatFromStr(r);
}

eImageFormat Hyprgraphics::formatFromStream(const std::span<const uint8_t>& data) {
    return formatOf(data);
}

eImageFormat Hyprgraphics::formatFromFile(const std::string& path) {
    return formatOf(path);
}

const char* Hyprgraphics::mimeOf(eImageFormat f) {
    switch (f) {
        case IMAGE_FORMAT_PNG: return "image/png";
        case IMAGE_FORMAT_AVIF: return "image/avif";
        case IMAGE_FORMAT_BMP: return "image/bmp";
        case IMAGE_FORMAT_JPEG: return "image/jpeg";
        case IMAGE_FORMAT_JXL: return "image/jxl";
        case IMAGE_FORMAT_SVG: return "image/svg";
        case IMAGE_FORMAT_WEBP: return "image/webp";
        default: return "error";
    }
}
