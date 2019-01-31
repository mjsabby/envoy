#pragma once

#include "common/common/thread_impl.h"
#include "common/filesystem/raw_instance_impl.h"

namespace Envoy {

class PlatformImpl {
public:
  Thread::ThreadFactory& threadFactory() { return thread_factory_; }
  Filesystem::RawInstance& rawFileSystem() { return raw_instance_; }

private:
  Thread::ThreadFactoryImplPosix thread_factory_;
  Filesystem::RawInstanceImplPosix raw_instance_;
};

} // namespace Envoy