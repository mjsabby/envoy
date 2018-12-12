#pragma once

#include "common/common/assert.h"
#include "common/common/thread_impl.h"

#include "absl/debugging/symbolize.h"

namespace Envoy {

class PlatformSetup {
public:
  PlatformSetup(int, const char* const* argv) {
#ifndef __APPLE__
    // absl::Symbolize mostly works without this, but this improves corner case
    // handling, such as running in a chroot jail.
    absl::InitializeSymbolizer(argv[0]);
#endif
  }

  Thread::ThreadFactory& threadFactory() { return thread_factory_; }

private:
  Thread::ThreadFactoryImplPosix thread_factory_;
  
};

} // namespace Envoy