#pragma once

namespace Hyprgraphics {
    class CColor {
      public:
        // SRGB (NOT linear!!) 0.0 - 1.0
        struct SSRGB {
            double r = 0, g = 0, b = 0;
        };

        // HSL 0.0 - 1.0
        struct SHSL {
            double h = 0, s = 0, l = 0;
        };

        // OkLab 0.0 - 1.0
        struct SOkLab {
            double l = 0, a = 0, b = 0;
        };

        CColor() = default; // black
        CColor(const SSRGB& rgb);
        CColor(const SHSL& hsl);
        CColor(const SOkLab& lab);

        SSRGB  asRgb() const;
        SHSL   asHSL() const;
        SOkLab asOkLab() const;

        bool operator==(const CColor& other) const {
            return other.r == r && other.g == g && other.b == b;
        }

      private:
        // SRGB space for internal color storage
        double r = 0, g = 0, b = 0;
    };
};
