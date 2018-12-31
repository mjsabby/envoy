#include "mocks.h"

#include "common/common/assert.h"
#include "common/common/lock_guard.h"

#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ReturnRef;

namespace Envoy {
namespace Api {

MockApi::MockApi()
    : thread_factory_(Thread::threadFactoryForTest()),
      file_system_(std::chrono::milliseconds(1000), thread_factory_, store_,
                   Filesystem::fileSystemForTest()) {
  ON_CALL(*this, threadFactory()).WillByDefault(ReturnRef(thread_factory_));
  ON_CALL(*this, fileSystem()).WillByDefault(ReturnRef(file_system_));
}

MockApi::~MockApi() {}

MockOsSysCalls::MockOsSysCalls() {}

MockOsSysCalls::~MockOsSysCalls() {}

SysCallIntResult MockOsSysCalls::setsockopt(SOCKET_FD sockfd, int level, int optname,
                                            const void* optval, socklen_t optlen) {
  ASSERT(optlen == sizeof(int));

  // Allow mocking system call failure.
  if (setsockopt_(sockfd, level, optname, optval, optlen) != 0) {
    return SysCallIntResult{-1, 0};
  }

  boolsockopts_[SockOptKey(sockfd, level, optname)] = !!*reinterpret_cast<const int*>(optval);
  return SysCallIntResult{0, 0};
};

SysCallIntResult MockOsSysCalls::getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                                            socklen_t* optlen) {
#if !defined(WIN32)
  ASSERT(*optlen == sizeof(int));
#else
  ASSERT(*optlen == sizeof(int) || *optlen == sizeof(WSAPROTOCOL_INFO));
#endif
  int val = 0;
  const auto& it = boolsockopts_.find(SockOptKey(sockfd, level, optname));
  if (it != boolsockopts_.end()) {
    val = it->second;
  }
  // Allow mocking system call failure.
  if (getsockopt_(sockfd, level, optname, optval, optlen) != 0) {
    return {-1, 0};
  }
  *reinterpret_cast<int*>(optval) = val;
  return {0, 0};
}

} // namespace Api
} // namespace Envoy
