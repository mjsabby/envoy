#pragma once

#include <functional>

#include "envoy/thread/thread.h"

namespace Envoy {
namespace Thread {

/**
 * Get current thread id.
 */
ThreadId currentThreadId();

/**
 * Wrapper for a pthread thread. We don't use std::thread because it eats exceptions and leads to
 * unusable stack traces.
 */
class ThreadImpl : public Thread {
public:
  ThreadImpl(std::function<void()> thread_routine);

  /**
   * Join on thread exit.
   */
  void join() override;
  ThreadHandle handle() const override { return thread_handle_; }

private:
  std::function<void()> thread_routine_;
  pthread_t thread_handle_;
};

} // namespace Thread
} // namespace Envoy
