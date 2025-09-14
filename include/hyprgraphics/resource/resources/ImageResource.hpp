#pragma once

#include "AsyncResource.hpp"
#include "../../color/Color.hpp"

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
        virtual ~CImageResource() = default;

        virtual void render();

      private:
        std::string m_path;
    };
};
