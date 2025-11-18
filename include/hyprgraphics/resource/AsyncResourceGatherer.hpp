#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include "../cairo/CairoSurface.hpp"
#include "./resources/AsyncResource.hpp"
#include <hyprutils/memory/Atomic.hpp>

namespace Hyprgraphics {
    class CAsyncResourceGatherer {
      public:
        CAsyncResourceGatherer();
        ~CAsyncResourceGatherer();

        void enqueue(Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource> resource);

        // Synchronously await the resource being available
        void await(Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource> resource);

      private:
        std::thread m_gatherThread;

        struct {
            std::mutex              requestMutex;
            std::condition_variable requestsCV;

            bool                    exit           = false;
            bool                    needsToProcess = false;
        } m_asyncLoopState;

        std::vector<Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource>> m_targetsToLoad;
        std::mutex                                                           m_targetsToLoadMutex;

        //
        void asyncAssetSpinLock();
        void wakeUpMainThread();
    };
}
