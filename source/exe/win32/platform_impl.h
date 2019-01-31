#pragma once

#include "common/common/assert.h"
#include "common/common/thread_impl.h"
#include "common/filesystem/raw_instance_impl.h"

// clang-format off
#include <winsock2.h>
// clang-format on

namespace Envoy {

class PlatformImpl {
public:
  PlatformImpl() {
    const WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    const int rc = ::WSAStartup(wVersionRequested, &wsaData);
    RELEASE_ASSERT(rc == 0, "WSAStartup failed with error");
  }

  ~PlatformImpl() { ::WSACleanup(); }

  Thread::ThreadFactory& threadFactory() { return thread_factory_; }
  Filesystem::RawInstance& rawFileSystem() { return raw_instance_; }

private:
  Thread::ThreadFactoryImplWin32 thread_factory_;
  Filesystem::RawInstanceImplWin32 raw_instance_;
};

} // namespace Envoy