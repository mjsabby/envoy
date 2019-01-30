#include "test/mocks/filesystem/mocks.h"

#include "common/common/lock_guard.h"

namespace Envoy {
namespace Filesystem {

MockRawFile::MockRawFile() : num_opens_(0), num_writes_(0), is_open_(false) {}
MockRawFile::~MockRawFile() {}

Api::SysCallBoolResult MockRawFile::open() {
  Thread::LockGuard lock(open_mutex_);

  const Api::SysCallBoolResult result = open_();
  is_open_ = result.rc_;
  num_opens_++;
  open_event_.notifyOne();

  return result;
}

Api::SysCallSizeResult MockRawFile::write(absl::string_view buffer) {
  Thread::LockGuard lock(write_mutex_);
  if (!is_open_) {
    return {-1, EBADF};
  }

  const Api::SysCallSizeResult result = write_(buffer);
  num_writes_++;
  write_event_.notifyOne();

  return result;
}

Api::SysCallBoolResult MockRawFile::close() {
  const Api::SysCallBoolResult result = close_();
  is_open_ = !result.rc_;

  return result;
}

MockRawInstance::MockRawInstance() {}
MockRawInstance::~MockRawInstance() {}

MockFile::MockFile() {}
MockFile::~MockFile() {}

MockInstance::MockInstance() {}
MockInstance::~MockInstance() {}

MockWatcher::MockWatcher() {}
MockWatcher::~MockWatcher() {}

} // namespace Filesystem
} // namespace Envoy
