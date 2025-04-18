#include "Png.hpp"
#include <spng.h>
#include <cstddef>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <hyprutils/utils/ScopeGuard.hpp>
using namespace Hyprutils::Utils;

static std::vector<unsigned char> readBinaryFile(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    if (!f.good())
        return {};
    f.unsetf(std::ios::skipws);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

std::expected<cairo_surface_t*, std::string> PNG::createSurfaceFromPNG(const std::string& path) {
    if (!std::filesystem::exists(path))
        return std::unexpected("loading png: file doesn't exist");

    spng_ctx*   ctx = spng_ctx_new(0);

    CScopeGuard x([&] { spng_ctx_free(ctx); });

    const auto  PNGCONTENT = readBinaryFile(path);

    if (PNGCONTENT.empty())
        return std::unexpected("loading png: file content was empty (bad file?)");

    spng_set_png_buffer(ctx, PNGCONTENT.data(), PNGCONTENT.size());

    spng_ihdr ihdr{.width = 0};
    if (int ret = spng_get_ihdr(ctx, &ihdr); ret)
        return std::unexpected(std::string{"loading png: spng_get_ihdr failed: "} + spng_strerror(ret));

    int fmt = SPNG_FMT_PNG;
    if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
        fmt = SPNG_FMT_RGB8;

    size_t imageLength = 0;
    if (int ret = spng_decoded_image_size(ctx, fmt, &imageLength); ret)
        return std::unexpected(std::string{"loading png: spng_decoded_image_size failed: "} + spng_strerror(ret));

    uint8_t* imageData = (uint8_t*)malloc(imageLength);

    if (!imageData)
        return std::unexpected("loading png: mallocing failed, out of memory?");

    // TODO: allow proper decode of high bitrate images
    bool succeededDecode = false;
    int  ret             = spng_decode_image(ctx, imageData, imageLength, SPNG_FMT_RGBA8, 0);
    if (!ret)
        succeededDecode = true;

    if (!succeededDecode && ret == SPNG_EBUFSIZ) {
        // hack, but I don't know why decoded_image_size is sometimes wrong
        imageLength = static_cast<size_t>(ihdr.height * ihdr.width * 4) /* FIXME: this is wrong if we doing >32bpp!!!! */;
        imageData   = (uint8_t*)realloc(imageData, imageLength);

        ret = spng_decode_image(ctx, imageData, imageLength, SPNG_FMT_RGBA8, 0);
    }

    if (!ret)
        succeededDecode = true;

    if (!succeededDecode) {
        free(imageData);
        return std::unexpected(std::string{"loading png: spng_decode_image failed: "} + spng_strerror(ret) + " (bad image?)");
    }

    // convert RGBA8888 -> ARGB8888 premult for cairo
    for (size_t i = 0; i < imageLength; i += 4) {
        uint8_t r, g, b, a;
        a = ((*((uint32_t*)(imageData + i))) & 0xFF000000) >> 24;
        b = ((*((uint32_t*)(imageData + i))) & 0x00FF0000) >> 16;
        g = ((*((uint32_t*)(imageData + i))) & 0x0000FF00) >> 8;
        r = (*((uint32_t*)(imageData + i))) & 0x000000FF;

        r *= ((float)a / 255.F);
        g *= ((float)a / 255.F);
        b *= ((float)a / 255.F);

        *((uint32_t*)(imageData + i)) = (((uint32_t)a) << 24) | (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | (uint32_t)b;
    }

    auto CAIROSURFACE = cairo_image_surface_create_for_data(imageData, CAIRO_FORMAT_ARGB32, ihdr.width, ihdr.height, ihdr.width * 4);

    if (!CAIROSURFACE)
        return std::unexpected("loading png: cairo failed");

    return CAIROSURFACE;
}
