#pragma once

#include "AsyncResource.hpp"
#include "../../color/Color.hpp"
#include "hyprgraphics/image/Image.hpp"

#include <optional>

#include <hyprutils/math/Vector2D.hpp>

namespace Hyprgraphics {
    class CImageResource : public IAsyncResource {
      public:
        enum eTextAlignmentMode : uint8_t {
            TEXT_ALIGN_LEFT = 0,
            TEXT_ALIGN_CENTER,
            TEXT_ALIGN_RIGHT,
        };

        CImageResource(const std::string& path);
        CImageResource(const std::span<const uint8_t> data, eImageFormat format);
        CImageResource(const std::string& svg, const Hyprutils::Math::Vector2D& size);
        virtual ~CImageResource() = default;

        virtual void render();

      private:
        std::string                    m_path;
        Hyprutils::Math::Vector2D      m_svgSize;

        const std::span<const uint8_t> m_data;
        const eImageFormat             m_format = eImageFormat::IMAGE_FORMAT_PNG;
    };
};
