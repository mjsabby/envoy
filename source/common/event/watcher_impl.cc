#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/event/dispatcher_impl.h"
#include "common/event/watcher_impl.h"

#include "uv.h"

namespace Envoy {
namespace Event {

WatcherImpl::~WatcherImpl() {
  for (FileWatch& file_watch : file_watches_) {
    const int rc = uv_fs_event_stop(&file_watch.handle_);
    ASSERT(rc == 0);
    uv_close(reinterpret_cast<uv_handle_t*>(&file_watch.handle_), nullptr);
  }
}

void WatcherImpl::addWatch(const std::string& path, OnChangedCb cb) {
  file_watches_.emplace_back(FileWatch(cb));
  FileWatch* file_watch = &file_watches_.back(); 
  file_watch->handle_.data = file_watch;
  int rc = uv_fs_event_init(loop_, &file_watch->handle_);
  ASSERT(rc == 0);

  auto fsEventCb = [](uv_fs_event_t* handle, const char* filename, int events, int status){
    FileWatch* file_watch = static_cast<FileWatch*>(handle->data);
    file_watch->cb_(filename, events, status);
  };

  rc = uv_fs_event_start(&file_watch->handle_, fsEventCb, path.c_str(), 0);
  if (rc != 0) {
    printf("fs_event_start: %s\n", uv_strerror(rc));
    throw EnvoyException("erroring");
  }
}

} // namespace Event
} // namespace Envoy
