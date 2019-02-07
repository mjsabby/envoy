#include "common/api/api_impl.h"

#include <chrono>
#include <string>

#include "common/common/thread.h"
#include "common/event/dispatcher_impl.h"

namespace Envoy {
namespace Api {

Impl::Impl(Thread::ThreadFactory& thread_factory, Stats::Store&, Event::TimeSystem& time_system,
           Filesystem::Instance& file_system_)
    : thread_factory_(thread_factory), time_system_(time_system), file_system_(file_system_) {}

Event::DispatcherPtr Impl::allocateDispatcher() {
  return std::make_unique<Event::DispatcherImpl>(*this);
}

} // namespace Api
} // namespace Envoy
