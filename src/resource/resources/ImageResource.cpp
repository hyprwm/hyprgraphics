#include <hyprgraphics/resource/resources/ImageResource.hpp>
#include <hyprgraphics/image/Image.hpp>
#include <hyprutils/memory/Atomic.hpp>
#include <hyprutils/memory/Casts.hpp>

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

CImageResource::CImageResource(const std::string& path) : m_path(path) {
    ;
}

void CImageResource::render() {
    auto image = CImage(m_path);

    m_asset.cairoSurface = image.cairoSurface();
    m_asset.pixelSize    = m_asset.cairoSurface && m_asset.cairoSurface->cairo() ? m_asset.cairoSurface->size() : Hyprutils::Math::Vector2D{};
}