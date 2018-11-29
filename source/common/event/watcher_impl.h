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
  WatcherImpl::WatcherImpl(DispatcherImpl& dispatcher) : loop_(&dispatcher.loop()) {}
  ~WatcherImpl();

  void addWatch(const std::string& path, OnChangedCb cb) override;

private:
  struct FileWatch {
    FileWatch(OnChangedCb cb) : cb_(cb) {}

    uv_fs_event_t handle_;
    OnChangedCb cb_;
  };

  std::list<FileWatch> file_watches_;
  uv_loop_t* loop_;
  OnChangedCb cb_;
};

} // namespace Event
} // namespace Envoy
