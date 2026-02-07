#pragma once

#include <string>
#include <span>
#include <hyprutils/math/Vector2D.hpp>
#include "./AsyncResource.hpp"
#include "../../color/Color.hpp"

#include <optional>
#include <span>

namespace Hyprgraphics {
    class CImageResource : public IAsyncResource {
      public:
        enum eTextAlignmentMode : uint8_t {
            TEXT_ALIGN_LEFT = 0,
            TEXT_ALIGN_CENTER,
            TEXT_ALIGN_RIGHT,
        };

        CImageResource(const std::string& path);
        CImageResource(const std::string& svg, const Hyprutils::Math::Vector2D& size);
        CImageResource(const std::span<const uint8_t>& data, const Hyprutils::Math::Vector2D& size = {} /* unused if not svg */);
        virtual ~CImageResource() = default;

        virtual void render();

      private:
        std::string               m_path;
        Hyprutils::Math::Vector2D m_svgSize;
        std::span<const uint8_t>  m_data;
    };
};
