#pragma once

#include <array>
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

        // xy 0.0 - 1.0
        struct xy {
            double x = 0, y = 0;

            bool   operator==(const xy& p2) const {
                return x == p2.x && y == p2.y;
            }
        };

        // XYZ 0.0 - 1.0
        struct XYZ {
            double x = 0, y = 0, z = 0;

            XYZ    operator/(const XYZ& other) const {
                return {x / other.x, y / other.y, z / other.z};
            }
        };

        CColor(); // black
        CColor(const SSRGB& rgb);
        CColor(const SHSL& hsl);
        CColor(const SOkLab& lab);

        SSRGB  asRgb() const;
        SHSL   asHSL() const;
        SOkLab asOkLab() const;

        bool   operator==(const CColor& other) const {
            return other.r == r && other.g == g && other.b == b;
        }

      private:
        // SRGB space for internal color storage
        double r = 0, g = 0, b = 0;
    };

    typedef std::array<double, 3> vec3;
    typedef std::array<vec3, 3>   mat3;

    class CMatrix3 {
      public:
        CMatrix3() = default;
        CMatrix3(const mat3& values);

        CMatrix3    invert();
        CColor::XYZ operator*(const CColor::XYZ& xyz) const;
        CMatrix3    operator*(const CMatrix3& other) const;

        const mat3& mat();

      private:
        mat3 m = {
            0, 0, 0, //
            0, 0, 0, //
            0, 0, 0, //
        };
    };

    const CMatrix3 Identity3 = CMatrix3(mat3{
        1, 0, 0, //
        0, 1, 0, //
        0, 0, 1, //
    });

    CColor::XYZ    xy2xyz(const CColor::xy& xy);
    CMatrix3       adaptWhite(const CColor::xy& src, const CColor::xy& dst);

    struct SPCPRimaries {
        CColor::xy red, green, blue, white;

        bool       operator==(const SPCPRimaries& p2) const {
            return red == p2.red && green == p2.green && blue == p2.blue && white == p2.white;
        }

        CMatrix3 toXYZ() const;
        CMatrix3 convertMatrix(const SPCPRimaries& dst) const;
    };
};
