#pragma once

#include <cairo/cairo.h>
#include <string>
#include <expected>
#include <png.h>

namespace PNG {
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const std::string&);
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const unsigned char*, size_t);
};

static std::expected<cairo_surface_t*, std::string> loadPNG(png_structp, png_infop);
