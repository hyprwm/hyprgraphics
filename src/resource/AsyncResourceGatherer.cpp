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
    {
        // under requestMutex: set between the gatherer's predicate check and its wait, the flag was missed
        // and the request sat until the 5 s wait_for timeout
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
    // The cv is created and the flag checked under awaitingMtx, and the gatherer sets the flag under the
    // same mutex unconditionally: a resource that finished before await() ran used to leave the flag unset
    // (no cv yet), and the caller then waited forever (lost wakeup -> compositor deadlock).
    std::unique_lock<std::mutex> lk(resource->m_impl->awaitingMtx);
    if (resource->m_impl->awaitingEvent)
        return;
    resource->m_impl->awaitingCv = Hyprutils::Memory::makeUnique<std::condition_variable>();
    resource->m_impl->awaitingCv->wait(lk, [&resource] { return resource->m_impl->awaitingEvent; });
    resource->m_impl->awaitingCv.reset();
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

            {
                std::lock_guard<std::mutex> lg(r->m_impl->awaitingMtx);
                r->m_impl->awaitingEvent = true;
                if (r->m_impl->awaitingCv)
                    r->m_impl->awaitingCv->notify_all();
            }
            r->m_ready = true;
            r->m_events.finished.emit();
        }
    }
}