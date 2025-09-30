#include "Svg.hpp"
#include <filesystem>
#include <fstream>
#include <librsvg/rsvg.h>
#include <hyprutils/utils/ScopeGuard.hpp>
#include <hyprutils/string/String.hpp>

using namespace Hyprutils::Utils;
using namespace Hyprutils::Math;
using namespace Hyprutils::String;

static std::optional<std::string> readFileAsString(const std::string& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec)
        return std::nullopt;

    std::ifstream file(path);
    if (!file.good())
        return std::nullopt;

    return trim(std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())));
}

std::expected<cairo_surface_t*, std::string> SVG::createSurfaceFromSVG(const std::string& path, const Vector2D& size) {
    if (!std::filesystem::exists(path))
        return std::unexpected("loading svg: file doesn't exist");

    if (size.x < 1 || size.y < 1)
        return std::unexpected("loading svg: invalid size");

    auto       cairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.x, size.y);

    const auto PCAIRO = cairo_create(cairoSurface);

    cairo_save(PCAIRO);
    cairo_set_operator(PCAIRO, CAIRO_OPERATOR_CLEAR);
    cairo_paint(PCAIRO);
    cairo_restore(PCAIRO);

    GError* error = nullptr;
    auto    file  = readFileAsString(path);

    if (!file)
        return std::unexpected("loading png: file doesn't exist / inaccessible");

    RsvgHandle* handle = rsvg_handle_new_from_data((unsigned char*)file->data(), file->size(), &error);

    if (!handle)
        return std::unexpected("loading svg: rsvg failed to read data");

    RsvgRectangle rect = {0, 0, (double)size.x, (double)size.y};

    if (!rsvg_handle_render_document(handle, PCAIRO, &rect, &error))
        return std::unexpected("loading svg: rsvg failed to render");

    // done
    cairo_surface_flush(cairoSurface);
    cairo_destroy(PCAIRO);
    g_object_unref(handle);

    return cairoSurface;
}