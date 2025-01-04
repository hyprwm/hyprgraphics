#include <print>
#include <format>
#include <filesystem>
#include <hyprgraphics/image/Image.hpp>
#include "shared.hpp"

using namespace Hyprgraphics;

bool tryLoadImage(const std::string& path) {
    auto image = CImage(path);

    if (!image.success()) {
        std::println("Failed to load {}: {}", path, image.getError());
        return false;
    }

    std::println("Loaded {} successfully: Image is {}x{} of type {}", path, image.cairoSurface()->size().x, image.cairoSurface()->size().y, image.getMime());

    return true;
}

int main(int argc, char** argv, char** envp) {
    int ret = 0;

    for (auto& file : std::filesystem::directory_iterator("./resource/images/")) {
        if (!file.is_regular_file())
            continue;
        auto expectation = true;
#ifndef JXL_FOUND
        if (file.path().filename() == "hyprland.jxl") expectation = false;
#endif
        EXPECT(tryLoadImage(file.path()), expectation);
    }

    return ret;
}
