#pragma once

#include <cairo/cairo.h>
#include <cstdint>
#include <span>
#include <string>
#include <expected>

namespace AVIF {
    std::expected<cairo_surface_t*, std::string> createSurfaceFromAvif(const std::string&);
    std::expected<cairo_surface_t*, std::string> createSurfaceFromAvif(const std::span<const uint8_t>);
};
