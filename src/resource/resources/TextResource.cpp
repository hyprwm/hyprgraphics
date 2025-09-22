#include <hyprgraphics/resource/resources/TextResource.hpp>
#include <hyprutils/memory/Atomic.hpp>
#include <hyprutils/memory/Casts.hpp>

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

CTextResource::CTextResource(CTextResource::STextResourceData&& data) : m_data(std::move(data)) {
    ;
}

void CTextResource::render() {
    auto                  CAIROSURFACE = makeUnique<CCairoSurface>(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1920, 1080 /* dummy value */));
    auto                  CAIRO        = cairo_create(CAIROSURFACE->cairo());

    PangoLayout*          layout = pango_cairo_create_layout(CAIRO);

    PangoFontDescription* fontDesc = pango_font_description_from_string(m_data.font.c_str());
    pango_font_description_set_size(fontDesc, m_data.fontSize * PANGO_SCALE);
    pango_layout_set_font_description(layout, fontDesc);
    pango_font_description_free(fontDesc);

    cairo_font_options_t* options = cairo_font_options_create();
    cairo_font_options_set_antialias(options, m_data.antialias);
    cairo_font_options_set_hint_style(options, m_data.hintStyle);
    pango_cairo_context_set_font_options(pango_layout_get_context(layout), options);
    cairo_font_options_destroy(options);

    PangoAlignment pangoAlign = PANGO_ALIGN_LEFT;

    switch (m_data.align) {
        case TEXT_ALIGN_LEFT: break;
        case TEXT_ALIGN_CENTER: pangoAlign = PANGO_ALIGN_CENTER; break;
        case TEXT_ALIGN_RIGHT: pangoAlign = PANGO_ALIGN_RIGHT; break;
        default: break;
    }

    pango_layout_set_alignment(layout, pangoAlign);

    PangoAttrList* attrList = nullptr;
    GError*        gError   = nullptr;
    char*          buf      = nullptr;
    if (pango_parse_markup(m_data.text.c_str(), -1, 0, &attrList, &buf, nullptr, &gError))
        pango_layout_set_text(layout, buf, -1);
    else {
        g_error_free(gError);
        pango_layout_set_text(layout, m_data.text.c_str(), -1);
    }

    if (!attrList)
        attrList = pango_attr_list_new();

    if (buf)
        free(buf);

    pango_attr_list_insert(attrList, pango_attr_scale_new(1));
    pango_layout_set_attributes(layout, attrList);
    pango_attr_list_unref(attrList);

    int layoutWidth, layoutHeight;
    pango_layout_get_size(layout, &layoutWidth, &layoutHeight);

    if (m_data.maxSize) {
        layoutWidth  = m_data.maxSize->x > 0 ? std::min(layoutWidth, sc<int>(m_data.maxSize->x * PANGO_SCALE)) : layoutWidth;
        layoutHeight = m_data.maxSize->y > 0 ? std::min(layoutHeight, sc<int>(m_data.maxSize->y * PANGO_SCALE)) : layoutHeight;
        pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
        if (m_data.maxSize->x >= 0)
            pango_layout_set_width(layout, layoutWidth);
        if (m_data.maxSize->y >= 0)
            pango_layout_set_height(layout, layoutHeight);
    }

    // TODO: avoid this?
    cairo_destroy(CAIRO);

    CAIROSURFACE.reset();

    m_asset.cairoSurface = makeShared<CCairoSurface>(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, layoutWidth / PANGO_SCALE, layoutHeight / PANGO_SCALE));
    CAIRO                = cairo_create(m_asset.cairoSurface->cairo());

    // clear the pixmap
    cairo_save(CAIRO);
    cairo_set_operator(CAIRO, CAIRO_OPERATOR_CLEAR);
    cairo_paint(CAIRO);
    cairo_restore(CAIRO);

    // render the thing
    const auto RGB = m_data.color.asRgb();
    cairo_set_source_rgba(CAIRO, RGB.r, RGB.g, RGB.b, 1.F);

    cairo_move_to(CAIRO, 0, 0);
    pango_cairo_show_layout(CAIRO, layout);

    g_object_unref(layout);

    cairo_surface_flush(m_asset.cairoSurface->cairo());

    m_asset.pixelSize = {layoutWidth / (double)PANGO_SCALE, layoutHeight / (double)PANGO_SCALE};

    cairo_destroy(CAIRO);
}