#pragma once

#include <cairo/cairo.h>
#include <string>
#include <expected>
#include <png.h>
#include <span>

namespace PNG {
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const std::string&);
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const std::span<u_int8_t>&);
};
