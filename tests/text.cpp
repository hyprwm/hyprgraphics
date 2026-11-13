#include <cstring>
#include <utility>
#include <hyprgraphics/resource/resources/TextResource.hpp>
#include "shared.hpp"

using namespace Hyprgraphics;

static bool sameRendering(CTextResource::STextResourceData first, CTextResource::STextResourceData second) {
    CTextResource firstResource(std::move(first));
    CTextResource secondResource(std::move(second));
    firstResource.render();
    secondResource.render();

    const auto FIRST  = firstResource.m_asset.cairoSurface;
    const auto SECOND = secondResource.m_asset.cairoSurface;

    if (FIRST->status() != CAIRO_STATUS_SUCCESS || SECOND->status() != CAIRO_STATUS_SUCCESS || FIRST->size().x <= 0 || FIRST->size().y <= 0 || SECOND->size().x <= 0 ||
        SECOND->size().y <= 0) {
        std::cerr << "Failed to render text for comparison\n";
        return false;
    }

    if (FIRST->size() != SECOND->size() || FIRST->stride() != SECOND->stride())
        return false;

    return std::memcmp(FIRST->data(), SECOND->data(), FIRST->stride() * static_cast<size_t>(FIRST->size().y)) == 0;
}

int main() {
    int                                    ret = 0;

    const CTextResource::STextResourceData PLAIN{.font = "Sans"};
    const CTextResource::STextResourceData BOLD{.text = "<b>Sample Text</b>", .font = "Sans"};
    const CTextResource::STextResourceData ITALIC{.text = "<i>Sample Text</i>", .font = "Sans"};
    const CTextResource::STextResourceData BOLD_ITALIC{.text = "<b><i>Sample Text</i></b>", .font = "Sans"};
    const CTextResource::STextResourceData SEMIBOLD{.text = "<span weight=\"semibold\">Sample Text</span>", .font = "Sans"};
    const CTextResource::STextResourceData OBLIQUE{.text = "<span style=\"oblique\">Sample Text</span>", .font = "Sans"};

    // Ensure missing fonts or identical fallback glyphs cannot hide the regression.
    EXPECT(sameRendering(PLAIN, BOLD), false);
    EXPECT(sameRendering(PLAIN, ITALIC), false);
    EXPECT(sameRendering(PLAIN, OBLIQUE), false);

    EXPECT(sameRendering({.font = "Sans Bold"}, BOLD), true);
    EXPECT(sameRendering({.font = "Sans Italic"}, ITALIC), true);
    EXPECT(sameRendering({.font = "Sans Bold Italic"}, BOLD_ITALIC), true);
    EXPECT(sameRendering({.font = "Sans SemiBold"}, SEMIBOLD), true);
    EXPECT(sameRendering({.font = "Sans Oblique"}, OBLIQUE), true);

    EXPECT(sameRendering({.font = "Sans", .weight = 400, .italic = false}, PLAIN), true);
    EXPECT(sameRendering({.font = "Sans", .weight = 700, .italic = true}, BOLD_ITALIC), true);
    EXPECT(sameRendering({.font = "Sans Bold Italic", .weight = 400}, ITALIC), true);
    EXPECT(sameRendering({.font = "Sans Bold Italic", .italic = false}, BOLD), true);
    EXPECT(sameRendering({.font = "Sans Bold Italic", .weight = 400, .italic = false}, PLAIN), true);
    EXPECT(sameRendering({.font = "Sans SemiBold", .weight = 700}, BOLD), true);
    EXPECT(sameRendering({.font = "Sans Bold", .italic = true}, BOLD_ITALIC), true);
    EXPECT(sameRendering({.font = "Sans Oblique", .weight = 400}, OBLIQUE), true);
    EXPECT(sameRendering({.font = "Sans Oblique", .italic = false}, PLAIN), true);
    EXPECT(sameRendering({.font = "Sans Oblique", .italic = true}, ITALIC), true);

    return ret;
}
