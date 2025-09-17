#pragma once

#include "AsyncResource.hpp"
#include "../../color/Color.hpp"

#include <optional>

#include <hyprutils/math/Vector2D.hpp>

namespace Hyprgraphics {
    class CTextResource : public IAsyncResource {
      public:
        enum eTextAlignmentMode : uint8_t {
            TEXT_ALIGN_LEFT = 0,
            TEXT_ALIGN_CENTER,
            TEXT_ALIGN_RIGHT,
        };

        struct STextResourceData {
            std::string                              text     = "Sample Text";
            std::string                              font     = "Sans Serif";
            size_t                                   fontSize = 16;
            CColor                                   color    = CColor{CColor::SSRGB{.r = 1.F, .g = 1.F, .b = 1.F}};
            eTextAlignmentMode                       align    = TEXT_ALIGN_LEFT;
            std::optional<Hyprutils::Math::Vector2D> maxSize  = std::nullopt;
        };

        CTextResource(STextResourceData&& data);
        virtual ~CTextResource() = default;

        virtual void render();

      private:
        STextResourceData m_data;
    };
};
