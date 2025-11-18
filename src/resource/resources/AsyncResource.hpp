#include <hyprgraphics/resource/resources/AsyncResource.hpp>

#include <condition_variable>

namespace Hyprgraphics {
    struct SAsyncResourceImpl {
        Hyprutils::Memory::CUniquePointer<std::condition_variable> awaitingCv;
        std::mutex                                                 awaitingMtx;
        bool                                                       awaitingEvent = false;
    };
}