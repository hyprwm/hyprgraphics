#include <hyprgraphics/resource/AsyncResourceGatherer.hpp>
#include "resources/AsyncResource.hpp"

using namespace Hyprgraphics;

CAsyncResourceGatherer::CAsyncResourceGatherer() {
    m_gatherThread = std::thread([this]() { asyncAssetSpinLock(); });
}

CAsyncResourceGatherer::~CAsyncResourceGatherer() {
    m_asyncLoopState.exit = true;
    wakeUpMainThread();

    if (m_gatherThread.joinable())
        m_gatherThread.join();
}

void CAsyncResourceGatherer::wakeUpMainThread() {
    // set under the lock the gatherer checks it with, or it can go to sleep right after missing it
    {
        std::lock_guard<std::mutex> lg(m_asyncLoopState.requestMutex);
        m_asyncLoopState.needsToProcess = true;
    }

    m_asyncLoopState.requestsCV.notify_all();
}

void CAsyncResourceGatherer::enqueue(Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource> resource) {
    {
        std::lock_guard<std::mutex> lg(m_targetsToLoadMutex);
        m_targetsToLoad.emplace_back(resource);
    }

    wakeUpMainThread();
}

void CAsyncResourceGatherer::await(Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource> resource) {
    std::unique_lock<std::mutex> lk(resource->m_impl->awaitingMtx);
    resource->m_impl->awaitingCv.wait(lk, [&resource] { return resource->m_impl->awaitingEvent; });
}

void CAsyncResourceGatherer::asyncAssetSpinLock() {
    while (!m_asyncLoopState.exit) {

        std::unique_lock lk(m_asyncLoopState.requestMutex);
        if (!m_asyncLoopState.needsToProcess) // avoid a lock if a thread managed to request something already since we .unlock()ed
            m_asyncLoopState.requestsCV.wait_for(lk, std::chrono::seconds(5), [this] { return m_asyncLoopState.needsToProcess; }); // wait for events

        if (m_asyncLoopState.exit)
            break;

        m_asyncLoopState.needsToProcess = false;
        lk.unlock();
        m_targetsToLoadMutex.lock();

        if (m_targetsToLoad.empty()) {
            m_targetsToLoadMutex.unlock();
            continue;
        }

        auto requests = m_targetsToLoad;
        m_targetsToLoad.clear();

        m_targetsToLoadMutex.unlock();

        // process requests
        for (auto& r : requests) {
            r->render();

            // the resource may finish before anyone awaits it, the flag keeps that from being lost
            {
                std::lock_guard<std::mutex> lg(r->m_impl->awaitingMtx);
                r->m_impl->awaitingEvent = true;
            }

            r->m_impl->awaitingCv.notify_all();
            r->m_ready = true;
            r->m_events.finished.emit();
        }
    }
}