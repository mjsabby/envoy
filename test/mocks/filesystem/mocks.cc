#include "test/mocks/filesystem/mocks.h"

#include <string>

#include "gtest/gtest.h"

using testing::_;
using testing::Return;

namespace Envoy {
namespace Filesystem {

MockFile::MockFile() {}
MockFile::~MockFile() {}

MockInstance::MockInstance() {}
MockInstance::~MockInstance() {}

MockStatsInstance::MockStatsInstance() {
  ON_CALL(*this, createFile(_, _, _)).WillByDefault(Return(file_));
  ON_CALL(*this, createFile(_, _, _, _)).WillByDefault(Return(file_));
}

MockStatsInstance::~MockStatsInstance() {}

MockWatcher::MockWatcher() {}
MockWatcher::~MockWatcher() {}

} // namespace Filesystem
} // namespace Envoy
