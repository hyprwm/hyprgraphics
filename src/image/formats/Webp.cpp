#include "Webp.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <webp/decode.h>
#include <vector>

std::expected<cairo_surface_t*, std::string> WEBP::createSurfaceFromWEBP(const std::string& path) {

    if (!std::filesystem::exists(path))
        return std::unexpected("loading webp: file doesn't exist");

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
    std::vector<uint8_t> bytes(file.tellg());
    file.seekg(0);
    file.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

    // now the WebP is in the memory

    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config))
        return std::unexpected("loading webp: WebPInitDecoderConfig failed");

    if (WebPGetFeatures(bytes.data(), bytes.size(), &config.input) != VP8_STATUS_OK)
        return std::unexpected("loading webp: file is not valid webp");

    const auto HEIGHT = config.input.height;
    const auto WIDTH  = config.input.width;

    auto       cairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    if (cairo_surface_status(cairoSurface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(cairoSurface);
        return std::unexpected("loading webp: cairo failed");
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    config.output.colorspace = MODE_bgrA;
#else
    config.output.colorspace = MODE_Argb;
#endif

    const auto CAIRODATA   = cairo_image_surface_get_data(cairoSurface);
    const auto CAIROSTRIDE = cairo_image_surface_get_stride(cairoSurface);

    config.options.no_fancy_upsampling = 1;
    config.output.u.RGBA.rgba          = CAIRODATA;
    config.output.u.RGBA.stride        = CAIROSTRIDE;
    // Really good cast!
    config.output.u.RGBA.size          = static_cast<size_t>(CAIROSTRIDE * HEIGHT);
    config.output.is_external_memory   = 1;
    config.output.width                = WIDTH;
    config.output.height               = HEIGHT;

    if (WebPDecode(bytes.data(), bytes.size(), &config) != VP8_STATUS_OK)
        return std::unexpected("loading webp: webp decode failed");

    cairo_surface_mark_dirty(cairoSurface);
    cairo_surface_set_mime_data(cairoSurface, "image/webp", bytes.data(), bytes.size(), nullptr, nullptr);

    WebPFreeDecBuffer(&config.output);

    return cairoSurface;
}
