#include <hyprgraphics/color/Color.hpp>
#include <algorithm>
#include <cmath>

using namespace Hyprgraphics;

static double gammaToLinear(const double in) {
    return in >= 0.04045 ? std::pow((in + 0.055) / 1.055, 2.4) : in / 12.92;
}

static double linearToGamma(const double in) {
    return in >= 0.0031308 ? (1.055 * std::pow(in, 0.41666666666)) - 0.055 : 12.92 * in;
}

static double hueToRgb(double p, double q, double t) {
    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1.0 / 6.0)
        return p + ((q - p) * 6.0 * t);
    if (t < 1.0 / 2.0)
        return q;
    if (t < 2.0 / 3.0)
        return p + ((q - p) * (2.0 / 3.0 - t) * 6.0);
    return p;
}

Hyprgraphics::CColor::CColor() {
    ;
}

Hyprgraphics::CColor::CColor(const SSRGB& rgb) : r(rgb.r), g(rgb.g), b(rgb.b) {
    ;
}

Hyprgraphics::CColor::CColor(const SHSL& hsl) {
    if (hsl.s <= 0) {
        r = hsl.l;
        g = hsl.l;
        b = hsl.l;
    } else {
        const double q = hsl.l < 0.5 ? hsl.l * (1.0 + hsl.s) : hsl.l + hsl.s - (hsl.l * hsl.s);
        const double p = (2.0 * hsl.l) - q;
        r              = hueToRgb(p, q, hsl.h + (1.0 / 3.0));
        g              = hueToRgb(p, q, hsl.h);
        b              = hueToRgb(p, q, hsl.h - (1.0 / 3.0));
    }
}

Hyprgraphics::CColor::CColor(const SOkLab& lab) {
    const double l = std::pow(lab.l + (lab.a * 0.3963377774) + (lab.b * 0.2158037573), 3);
    const double m = std::pow(lab.l + (lab.a * (-0.1055613458)) + (lab.b * (-0.0638541728)), 3);
    const double s = std::pow(lab.l + (lab.a * (-0.0894841775)) + (lab.b * (-1.2914855480)), 3);

    r = linearToGamma((l * 4.0767416621) + (m * -3.3077115913) + (s * 0.2309699292));
    g = linearToGamma((l * (-1.2684380046)) + (m * 2.6097574011) + (s * (-0.3413193965)));
    b = linearToGamma((l * (-0.0041960863)) + (m * (-0.7034186147)) + (s * 1.7076147010));
}

Hyprgraphics::CColor::SSRGB Hyprgraphics::CColor::asRgb() const {
    return Hyprgraphics::CColor::SSRGB{
        .r = r,
        .g = g,
        .b = b,
    };
}

Hyprgraphics::CColor::SHSL Hyprgraphics::CColor::asHSL() const {
    const double vmax = std::max({r, g, b}), vmin = std::min({r, g, b});
    double       h = 0, s = 0, l = (vmax + vmin) / 2.0;

    if (vmax == vmin) {
        return Hyprgraphics::CColor::SHSL{
            .h = 0,
            .s = 0,
            .l = l,
        };
    }

    const double d = vmax - vmin;
    s              = l > 0.5 ? d / (2.0 - vmax - vmin) : d / (vmax + vmin);

    if (vmax == r)
        h = (g - b) / d + (g < b ? 6.0 : 0.0);
    if (vmax == g)
        h = (b - r) / d + 2;
    if (vmax == b)
        h = (r - g) / d + 4;

    h /= 6.0;

    return Hyprgraphics::CColor::SHSL{
        .h = h,
        .s = s,
        .l = l,
    };
}

Hyprgraphics::CColor::SOkLab Hyprgraphics::CColor::asOkLab() const {
    const double linR = gammaToLinear(r);
    const double linG = gammaToLinear(g);
    const double linB = gammaToLinear(b);

    const double l = std::cbrtf((0.4122214708 * linR) + (0.5363325363 * linG) + (0.0514459929 * linB));
    const double m = std::cbrtf((0.2119034982 * linR) + (0.6806995451 * linG) + (0.1073969566 * linB));
    const double s = std::cbrtf((0.0883024619 * linR) + (0.2817188376 * linG) + (0.6299787005 * linB));

    return Hyprgraphics::CColor::SOkLab{
        .l = (l * 0.2104542553) + (m * 0.7936177850) + (s * (-0.0040720468)),
        .a = (l * 1.9779984951) + (m * (-2.4285922050)) + (s * 0.4505937099),
        .b = (l * 0.0259040371) + (m * 0.7827717662) + (s * (-0.8086757660)),
    };
}
