#include "Png.hpp"
#include <cstddef>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <png.h>
#include <hyprutils/utils/ScopeGuard.hpp>
using namespace Hyprutils::Utils;

std::expected<cairo_surface_t*, std::string> PNG::createSurfaceFromPNG(const std::string& path) {
    if (!std::filesystem::exists(path))
        return std::unexpected("loading png: file doesn't exist");

    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return std::unexpected("loading png: couldn't open file");

    png_structp png  = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop   info = png_create_info_struct(png);
    if (!png || !info)
        return std::unexpected("loading png: couldn't init libpng");

    CScopeGuard x([&png, &info, fp] {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
    });

    if (setjmp(png_jmpbuf(png)))
        return std::unexpected("loading png: couldn't setjmp");

    png_init_io(png, fp);
    png_read_info(png, info);

    const size_t W          = png_get_image_width(png, info);
    const size_t H          = png_get_image_height(png, info);
    const auto   COLOR_TYPE = png_get_color_type(png, info);
    const auto   BPP        = png_get_bit_depth(png, info);

    if (BPP == 16)
        png_set_strip_16(png);
    if (COLOR_TYPE == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (COLOR_TYPE == PNG_COLOR_TYPE_GRAY && BPP < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (COLOR_TYPE == PNG_COLOR_TYPE_RGB || COLOR_TYPE == PNG_COLOR_TYPE_GRAY || COLOR_TYPE == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    else if (COLOR_TYPE == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    std::vector<uint8_t*> rowPointers;
    rowPointers.resize(H);
    std::vector<uint8_t> rawData;
    rawData.resize(W * H * 4);
    for (size_t y = 0; y < H; y++) {
        rowPointers[y] = rawData.data() + (y * W * 4);
    }

    png_read_image(png, rowPointers.data());

    for (size_t i = 0; i < W * H * 4; i += 4) {
        uint8_t r               = rawData[i + 0];
        uint8_t g               = rawData[i + 1];
        uint8_t b               = rawData[i + 2];
        uint8_t a               = rawData[i + 3];
        *(uint32_t*)&rawData[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    auto CAIROSURFACE = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);

    if (!CAIROSURFACE)
        return std::unexpected("loading png: cairo failed");

    memcpy(cairo_image_surface_get_data(CAIROSURFACE), rawData.data(), rawData.size());

    return CAIROSURFACE;
}
