#include <hyprgraphics/resource/resources/AsyncResource.hpp>

#include <condition_variable>
#include <mutex>

namespace Hyprgraphics {
    struct SAsyncResourceImpl {
        // awaitingEvent is only touched under awaitingMtx, so await() can't miss it
        std::condition_variable awaitingCv;
        std::mutex              awaitingMtx;
        bool                    awaitingEvent = false;
    };
}