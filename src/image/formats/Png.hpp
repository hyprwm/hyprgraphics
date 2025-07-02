#pragma once

#include <cairo/cairo.h>
#include <string>
#include <expected>
#include <png.h>
#include <span>
#include <cstdint>

namespace PNG {
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const std::string&);
    std::expected<cairo_surface_t*, std::string> createSurfaceFromPNG(const std::span<uint8_t>&);
};
