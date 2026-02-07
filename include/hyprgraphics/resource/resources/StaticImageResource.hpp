#pragma once

#include "./AsyncResource.hpp"
#include "../../color/Color.hpp"
#include "hyprgraphics/image/Image.hpp"

#include <optional>
#include <span>

#include <hyprutils/math/Vector2D.hpp>

namespace Hyprgraphics {
    class CStaticImageResource : public IAsyncResource {
      public:
        enum eTextAlignmentMode : uint8_t {
            TEXT_ALIGN_LEFT = 0,
            TEXT_ALIGN_CENTER,
            TEXT_ALIGN_RIGHT,
        };

        CStaticImageResource(const std::span<const uint8_t> data, eImageFormat format);
        virtual ~CStaticImageResource() = default;

        virtual void render();

      private:
        const std::span<const uint8_t> m_data;
        const eImageFormat             m_format = eImageFormat::IMAGE_FORMAT_PNG;
    };
};
