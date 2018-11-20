#pragma once

#include <cstdint>
#include <list>
#include <string>

#include "envoy/event/watcher.h"

#include "common/common/logger.h"
#include "common/event/dispatcher_impl.h"

namespace Envoy {
namespace Event {

/**
 * Implementation of Watcher that uses kqueue. If the file being watched doesn't exist, we watch
 * the directory, and then try to add a file watch each time there's a write event to the
 * directory.
 */
class WatcherImpl : public Watcher, Logger::Loggable<Logger::Id::file> {
public:
  WatcherImpl(Event::DispatcherImpl& dispatcher, const std::string& path, OnChangedCb cb);
  ~WatcherImpl();

private:
  uv_any_handle raw_handle_;
  uv_loop_t* loop_;
  OnChangedCb cb_;
};

} // namespace Event
} // namespace Envoy
