#include <hyprgraphics/resource/AsyncResourceGatherer.hpp>

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
    m_asyncLoopState.needsToProcess = true;
    m_asyncLoopState.requestsCV.notify_all();
}

void CAsyncResourceGatherer::enqueue(Hyprutils::Memory::CAtomicSharedPointer<IAsyncResource> resource) {
    {
        std::lock_guard<std::mutex> lg(m_targetsToLoadMutex);
        m_targetsToLoad.emplace_back(resource);
    }

    wakeUpMainThread();
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

            r->m_ready = true;
            r->m_events.finished.emit();
        }
    }
}