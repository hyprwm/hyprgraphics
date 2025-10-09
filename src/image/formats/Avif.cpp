#include "Avif.hpp"
#include <cairo.h>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <hyprutils/utils/ScopeGuard.hpp>
#include <libheif/heif.h>
#include <vector>
using namespace Hyprutils::Utils;

static std::expected<cairo_surface_t*, std::string> loadFromContext(heif_context* ctx) {
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

    heif_image*       img;
    struct heif_error err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr);

    if (err.code != heif_error_Ok)
        return std::unexpected("loading avif: failed to decode image");

    size_t width  = heif_image_get_width(img, heif_channel_interleaved);
    size_t height = heif_image_get_height(img, heif_channel_interleaved);

    if (width == static_cast<size_t>(-1) || height == static_cast<size_t>(-1))
        return std::unexpected("loading avif: failed to get width or height");

    int            stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    if (!data)
        return std::unexpected("loading avif: get_plane_readonly failed");

    std::vector<uint8_t> rawData;
    rawData.resize(width * height * 4);

    for (size_t y = 0; y < height; y++) {
        const uint8_t* src = data + (y * stride);
        uint32_t*      dst = (uint32_t*)(rawData.data() + (y * width * 4));
        for (size_t x = 0; x < width; x++) {
            uint8_t r = src[(4 * x) + 0];
            uint8_t g = src[(4 * x) + 1];
            uint8_t b = src[(4 * x) + 2];
            uint8_t a = src[(4 * x) + 3];

            r = (r * a) / 255.F;
            g = (g * a) / 255.F;
            b = (b * a) / 255.F;

            dst[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    auto CAIROSURFACE = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (!CAIROSURFACE)
        return std::unexpected("loading avif: cairo failed");

    memcpy(cairo_image_surface_get_data(CAIROSURFACE), rawData.data(), rawData.size());
    cairo_surface_mark_dirty(CAIROSURFACE);

    heif_image_release(img);
    heif_image_handle_release(handle);
    return CAIROSURFACE;
}

std::expected<cairo_surface_t*, std::string> AVIF::createSurfaceFromAvif(const std::string& path) {
    if (!std::filesystem::exists(path))
        return std::unexpected("loading avif: file doesn't exist");

    heif_context*     ctx = heif_context_alloc();
    struct heif_error err = heif_context_read_from_file(ctx, path.c_str(), nullptr);

    if (err.code != heif_error_Ok)
        return std::unexpected("loading avif: failed to load from file");

    auto result = loadFromContext(ctx);
    heif_context_free(ctx);

    return result;
}
std::expected<cairo_surface_t*, std::string> AVIF::createSurfaceFromAvif(const std::span<const uint8_t> buf) {
    heif_context*     ctx = heif_context_alloc();
    struct heif_error err = heif_context_read_from_memory(ctx, buf.data(), buf.size(), nullptr);

    if (err.code != heif_error_Ok)
        return std::unexpected("loading avif: failed to load from memory");

    auto result = loadFromContext(ctx);
    heif_context_free(ctx);

    return result;
}
