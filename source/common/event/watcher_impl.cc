#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/event/dispatcher_impl.h"
#include "common/event/watcher_impl.h"

#include "uv.h"

namespace Envoy {
namespace Event {

WatcherImpl::WatcherImpl(DispatcherImpl& dispatcher, const std::string& path, OnChangedCb cb) 
   : loop_(&dispatcher.loop()), cb_(cb) {
  raw_handle_.fs_event.data = this;

  int rc = uv_fs_event_init(loop_, &raw_handle_.fs_event);
  ASSERT(rc == 0);

  auto fsEventCb = [](uv_fs_event_t* handle, const char* filename, int events, int status){
    WatcherImpl* watcher = static_cast<WatcherImpl*>(handle->data);
    watcher->cb_(filename, events, status);
  };

  rc = uv_fs_event_start(&raw_handle_.fs_event, fsEventCb, path.c_str(), 0);
  ASSERT(rc == 0);
}

WatcherImpl::~WatcherImpl() {}

} // namespace Event
} // namespace Envoy
