#pragma once

#include <windows.h>

// <winsock2.h> includes <windows.h>, so undef some interfering symbols.
#undef TRUE
#undef DELETE
#undef ERROR
#undef GetMessage

#include <functional>

#include "envoy/thread/thread.h"

namespace Envoy {
namespace Thread {

/**
 * Wrapper for a win32 thread. We don't use std::thread because it eats exceptions and leads to
 * unusable stack traces.
 */
class ThreadImplWin32 : public Thread {
public:
  ThreadImplWin32(std::function<void()> thread_routine);
  ~ThreadImplWin32();

  // Thread::Thread
  void join() override;

  // Needed for the WIN32 implementation of WatcherImpl. TODO(YAEL) - PR with that implementation
  HANDLE handle() const { return thread_handle_; }

private:
  std::function<void()> thread_routine_;
  HANDLE thread_handle_;
};

/**
 * Implementation of ThreadFactory
 */
class ThreadFactoryImplWin32 : public ThreadFactory {
public:
  // Thread::ThreadFactory
  ThreadPtr createThread(std::function<void()> thread_routine) override;
  ThreadId currentThreadId() override;
};

} // namespace Thread
} // namespace Envoy
