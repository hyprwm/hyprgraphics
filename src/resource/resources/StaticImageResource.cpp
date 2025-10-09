#include <hyprgraphics/resource/resources/StaticImageResource.hpp>
#include <hyprgraphics/image/Image.hpp>
#include <hyprutils/memory/Atomic.hpp>
#include <hyprutils/memory/Casts.hpp>

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

CStaticImageResource::CStaticImageResource(const std::span<const uint8_t> data, eImageFormat format) : m_data(data), m_format(format) {
    ;
}

void CStaticImageResource::render() {
    auto image = CImage(m_data, m_format);

    m_asset.cairoSurface = image.cairoSurface();
    m_asset.pixelSize    = m_asset.cairoSurface && m_asset.cairoSurface->cairo() ? m_asset.cairoSurface->size() : Hyprutils::Math::Vector2D{};
}
