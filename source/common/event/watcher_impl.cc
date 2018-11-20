#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/event/watcher_impl.h"

#include "uv.h"

namespace Envoy {
namespace Event {

WatcherImpl::~WatcherImpl() {}

void WatcherImpl::addWatch(const std::string& path, uint32_t events, Watcher::OnChangedCb cb) {
  file_events_.emplace_back(uv_fs_event_t{});
}

} // namespace Event
} // namespace Envoy
