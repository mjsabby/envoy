#pragma once

#include <pthread.h>

#include <functional>

#include "envoy/thread/thread.h"

namespace Envoy {
namespace Thread {

class ThreadIdImplPosix : public ThreadId {
public:
  ThreadIdImplPosix(int64_t id);

  // Thread::ThreadId
  std::string debugString() const override;
  bool isCurrentThreadId() const override;

private:
  int64_t id_;
};

/**
 * Wrapper for a pthread thread. We don't use std::thread because it eats exceptions and leads to
 * unusable stack traces.
 */
class ThreadImplPosix : public Thread {
public:
  ThreadImplPosix(std::function<void()> thread_routine);

  // Thread::Thread
  void join() override;

private:
  std::function<void()> thread_routine_;
  pthread_t thread_handle_;
};

/**
 * Implementation of ThreadFactory
 */
class ThreadFactoryImplPosix : public ThreadFactory {
public:
  ThreadFactoryImplPosix() {}

  ThreadPtr createThread(std::function<void()> thread_routine) override;

  ThreadIdPtr currentThreadId() override;
};

} // namespace Thread
} // namespace Envoy