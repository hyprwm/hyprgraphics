#pragma once

#include <cairo/cairo.h>
#include <string>
#include <expected>
#include <png.h>
#include <hyprutils/math/Vector2D.hpp>

namespace SVG {
    std::expected<cairo_surface_t*, std::string> createSurfaceFromSVG(const std::string&, const Hyprutils::Math::Vector2D& size);
};
