#include <algorithm>
#include <print>
#include <format>
#include <filesystem>
#include <fstream>
#include <vector>
#include <hyprgraphics/resource/AsyncResourceGatherer.hpp>
#include <hyprgraphics/resource/resources/TextResource.hpp>
#include <hyprgraphics/resource/resources/ImageResource.hpp>
#include <hyprutils/memory/UniquePtr.hpp>
#include <hyprutils/memory/Atomic.hpp>
#include <hyprutils/math/Vector2D.hpp>
#include "shared.hpp"

using namespace Hyprutils::Memory;
using namespace Hyprutils::Math;
using namespace Hyprgraphics;

#define UP CUniquePointer

static UP<CAsyncResourceGatherer> g_asyncResourceGatherer;

static struct {
    std::mutex                                        wakeupMutex;
    std::condition_variable                           wakeup;

    bool                                              exit           = false;
    bool                                              needsToProcess = false;

    int                                               loadedAssets = 0;

    std::mutex                                        resourcesMutex;
    std::vector<CAtomicSharedPointer<IAsyncResource>> resources;
} state;

//

static bool renderText(const std::string& text, Vector2D max = {}) {
    // this stinks a bit but it's due to our ASP impl.
    auto resource =
        makeAtomicShared<CTextResource>(CTextResource::STextResourceData{.text = text, .fontSize = 72, .maxSize = max.x == 0 ? std::nullopt : std::optional<Vector2D>(max)});
    CAtomicSharedPointer<IAsyncResource> resourceGeneric(resource);

    g_asyncResourceGatherer->enqueue(resourceGeneric);

    state.resourcesMutex.lock();
    state.resources.emplace_back(std::move(resourceGeneric));
    state.resourcesMutex.unlock();

    resource->m_events.finished.listenStatic([]() {
        state.needsToProcess = true;
        state.wakeup.notify_all();
    });

    std::println("Enqueued \"{}\" successfully.", text);

    return true;
}

static bool renderImage(const std::string& path) {
    // this stinks a bit but it's due to our ASP impl.
    auto                                 resource = makeAtomicShared<CImageResource>(path);
    CAtomicSharedPointer<IAsyncResource> resourceGeneric(resource);

    g_asyncResourceGatherer->enqueue(resourceGeneric);

    state.resourcesMutex.lock();
    state.resources.emplace_back(std::move(resourceGeneric));
    state.resourcesMutex.unlock();

    resource->m_events.finished.listenStatic([]() {
        state.needsToProcess = true;
        state.wakeup.notify_all();
    });

    std::println("Enqueued \"{}\" successfully.", path);

    return true;
}

int main(int argc, char** argv, char** envp) {
    int ret = 0;

    g_asyncResourceGatherer = makeUnique<CAsyncResourceGatherer>();

    EXPECT(renderText("Hello World"), true);
    EXPECT(renderText("<b><i>Test markup</i></b>"), true);
    EXPECT(renderText("Test ellipsis!!!!!", {512, 190}),
           true);
    EXPECT(renderImage("./resource/images/hyprland.png"), true);

    while (!state.exit) {
        std::unique_lock lk(state.wakeupMutex);
        if (!state.needsToProcess) // avoid a lock if a thread managed to request something already since we .unlock()ed
            state.wakeup.wait_for(lk, std::chrono::seconds(5), [] { return state.needsToProcess; }); // wait for events

        if (state.exit)
            break;

        state.needsToProcess = false;

        state.resourcesMutex.lock();

        const bool SHOULD_EXIT = std::ranges::all_of(state.resources, [](const auto& e) { return !!e->m_ready; });

        state.resourcesMutex.unlock();

        if (SHOULD_EXIT)
            break;

        lk.unlock();
    }

    // all assets should be done, let's render them
    size_t idx = 0;
    for (const auto& r : state.resources) {
        const auto TEST_DIR = std::filesystem::current_path().string() + "/test_output";

        // try to write it for inspection
        if (!std::filesystem::exists(TEST_DIR))
            std::filesystem::create_directory(TEST_DIR);

        std::string name = std::format("render-arg-{}", idx);

        EXPECT(!!r->m_asset.cairoSurface->cairo(), true);

        //NOLINTNEXTLINE
        if (!r->m_asset.cairoSurface->cairo())
            continue;

        EXPECT(cairo_surface_write_to_png(r->m_asset.cairoSurface->cairo(), (TEST_DIR + "/" + name + ".png").c_str()), CAIRO_STATUS_SUCCESS);

        idx++;
    }

    g_asyncResourceGatherer.reset();

    return ret;
}
