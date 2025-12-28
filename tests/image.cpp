#include <algorithm>
#include <print>
#include <format>
#include <filesystem>
#include <fstream>
#include <vector>
#include <hyprgraphics/image/Image.hpp>
#include "shared.hpp"

using namespace Hyprgraphics;

static bool tryLoadImageFromFile(const std::string& path) {
    auto image = path.ends_with("svg") ? CImage(path, {512, 512}) : CImage(path);

    if (!image.success()) {
        std::println("Failed to load {}: {}", path, image.getError());
        return false;
    }

    std::println("Loaded {} successfully: Image is {}x{} of type {}", path, image.cairoSurface()->size().x, image.cairoSurface()->size().y, image.getMime());

    const auto TEST_DIR = std::filesystem::current_path().string() + "/test_output";

    // try to write it for inspection
    if (!std::filesystem::exists(TEST_DIR))
        std::filesystem::create_directory(TEST_DIR);

    std::string name = image.getMime();
    std::ranges::replace(name, '/', '_');

    //NOLINTNEXTLINE
    return cairo_surface_write_to_png(image.cairoSurface()->cairo(), (TEST_DIR + "/" + name + ".png").c_str()) == CAIRO_STATUS_SUCCESS;
}

static bool tryLoadImageFromBuffer(const std::span<uint8_t>& data, eImageFormat format) {
    auto image = CImage(data, format);

    if (!image.success()) {
        std::println("Failed to load embedded image: {}", image.getError());
        return false;
    }

    std::println("Loaded embedded Image successfully: Image is {}x{} of type {}", image.cairoSurface()->size().x, image.cairoSurface()->size().y, image.getMime());

    const auto TEST_DIR = std::filesystem::current_path().string() + "/test_output";

    // try to write it for inspection
    if (!std::filesystem::exists(TEST_DIR))
        std::filesystem::create_directory(TEST_DIR);

    std::string name = image.getMime() + "_embedded";
    std::ranges::replace(name, '/', '_');

    //NOLINTNEXTLINE
    return cairo_surface_write_to_png(image.cairoSurface()->cairo(), (TEST_DIR + "/" + name + ".png").c_str()) == CAIRO_STATUS_SUCCESS;
}

static std::vector<uint8_t> getImageBuffer(const std::string& path) {
    std::vector<uint8_t> buffer;

    std::ifstream        file(path, std::ios::binary | std::ios::ate);
    std::streamsize      size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.resize(size);

    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}

int main(int argc, char** argv, char** envp) {
    int ret = 0;

    for (auto& file : std::filesystem::directory_iterator("./resource/images/")) {
        if (!file.is_regular_file())
            continue;
        auto expectation = true;
#ifndef JXL_FOUND
        if (file.path().filename() == "hyprland.jxl")
            expectation = false;
#endif
#ifndef HEIF_FOUND
        if (file.path().filename() == "hyprland.avif")
            expectation = false;
#endif
        EXPECT(tryLoadImageFromFile(file.path()), expectation);
    }

    auto pngBuffer = getImageBuffer("./resource/images/hyprland.png");
    EXPECT(tryLoadImageFromBuffer(pngBuffer, Hyprgraphics::IMAGE_FORMAT_AUTO), true);

#ifdef HEIF_FOUND
    auto avifBuffer = getImageBuffer("./resource/images/hyprland.avif");
    EXPECT(tryLoadImageFromBuffer(avifBuffer, Hyprgraphics::IMAGE_FORMAT_AVIF), true);
#endif

    auto svgBuffer = getImageBuffer("./resource/images/hyprland.svg");
    EXPECT(tryLoadImageFromBuffer(pngBuffer, Hyprgraphics::IMAGE_FORMAT_AUTO), true);

    return ret;
}
