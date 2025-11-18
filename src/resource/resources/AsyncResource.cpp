#include "AsyncResource.hpp"

using namespace Hyprgraphics;
using namespace Hyprutils::Memory;

IAsyncResource::IAsyncResource() : m_impl(makeUnique<SAsyncResourceImpl>()) {
    ;
}