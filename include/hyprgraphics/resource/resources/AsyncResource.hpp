#pragma once

#include <hyprutils/memory/UniquePtr.hpp>
#include <hyprutils/math/Vector2D.hpp>
#include <hyprutils/signal/Signal.hpp>
#include "../../cairo/CairoSurface.hpp"
#include <atomic>

namespace Hyprgraphics {
    struct SAsyncResourceImpl;

    class IAsyncResource {
      public:
        IAsyncResource();
        virtual ~IAsyncResource() = default;

        virtual void render() = 0;

        struct {
            // this signal fires on the worker thread. **Really** consider making this signal handler call something to wake your
            // main event loop up and do things there.
            Hyprutils::Signal::CSignalT<> finished;
        } m_events;

        // you probably shouldn't use this but it's here just in case.
        std::atomic<bool> m_ready = false;

        struct {
            // This pointer can be made not thread safe as after .finished the worker thread will not touch it anymore
            // and before that you shouldnt touch it either
            Hyprutils::Memory::CSharedPointer<CCairoSurface> cairoSurface;
            Hyprutils::Math::Vector2D                        pixelSize;
        } m_asset;

        Hyprutils::Memory::CUniquePointer<SAsyncResourceImpl> m_impl;
    };
}
