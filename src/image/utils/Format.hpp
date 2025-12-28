#pragma once

#include <hyprgraphics/image/Image.hpp>

namespace Hyprgraphics {
    eImageFormat formatFromStream(const std::span<const uint8_t>& data);
    eImageFormat formatFromFile(const std::string& path);
    const char*  mimeOf(eImageFormat);
};
