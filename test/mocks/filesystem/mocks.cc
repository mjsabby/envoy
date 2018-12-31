#include "test/mocks/filesystem/mocks.h"

#include <string>

#include "common/common/lock_guard.h"

#include "gtest/gtest.h"

using testing::_;
using testing::Return;

namespace Envoy {
namespace Filesystem {

MockFile::MockFile() {}
MockFile::~MockFile() {}

void MockFile::open() {
  Thread::LockGuard lock(open_mutex_);

  open_();
  num_open_++;
  open_event_.notifyOne();

  return;
}

Api::SysCallSizeResult MockFile::write(const void* buffer, size_t num_bytes) {
  Thread::LockGuard lock(write_mutex_);

  Api::SysCallSizeResult rc = write_(buffer, num_bytes);
  num_writes_++;
  write_event_.notifyOne();

  return rc;
}

MockInstance::MockInstance() {}
MockInstance::~MockInstance() {}

MockStatsFile::MockStatsFile() {}
MockStatsFile::~MockStatsFile() {}

MockStatsInstance::MockStatsInstance() {}

MockStatsInstance::~MockStatsInstance() {}

MockWatcher::MockWatcher() {}
MockWatcher::~MockWatcher() {}

} // namespace Filesystem
} // namespace Envoy
