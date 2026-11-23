#include <hyprgraphics/resource/AsyncResourceGatherer.hpp>
#include <hyprgraphics/resource/resources/AsyncResource.hpp>
#include <hyprutils/memory/Atomic.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <future>

#include "shared.hpp"

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

// renders instantly, so the gatherer often finishes before await() starts waiting
class CInstantResource : public IAsyncResource {
  public:
    virtual void render() {
        ;
    }
};

int main() {
    int                    ret = 0;

    CAsyncResourceGatherer gatherer;

    // the slowest await, a missed wakeup only recovers through the gatherer's 5s poll
    auto       awaited = std::async(std::launch::async, [&gatherer] {
        std::chrono::steady_clock::duration slowest{};
        for (size_t i = 0; i < 20000; ++i) {
            const auto                           INSTANT = makeAtomicShared<CInstantResource>();
            CAtomicSharedPointer<IAsyncResource> resource(INSTANT);

            const auto                           START = std::chrono::steady_clock::now();
            gatherer.enqueue(resource);
            gatherer.await(resource);
            slowest = std::max(slowest, std::chrono::steady_clock::now() - START);
        }

        return slowest;
    });

    const bool FINISHED = awaited.wait_for(std::chrono::seconds(30)) == std::future_status::ready;
    EXPECT(FINISHED, true);

    // a lost wakeup leaves await() blocked forever, that thread can't be joined
    if (!FINISHED) {
        std::cout << std::flush;
        std::_Exit(1);
    }

    EXPECT(awaited.get() < std::chrono::seconds(1), true);

    return ret;
}
