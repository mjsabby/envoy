#pragma once

#include <functional>

#include "envoy/thread/thread.h"

#if defined(WIN32)
#include <windows.h>
// <windows.h> defines some macros that interfere with our code, so undef them
#undef DELETE
#undef GetMessage
#endif

namespace Envoy {
namespace Thread {

class ThreadIdImplWin32 : public ThreadId {
public:
  ThreadIdImplWin32(DWORD id) : id_(id) {}

  std::string string() const override {
    return std::to_string(id_);
  }

  bool operator==(const ThreadId& rhs) const override {
    return id_ == dynamic_cast<const ThreadIdImplWin32&>(rhs).id_;
  }

  bool isCurrentThreadId() const override {
    return id_ == ::GetCurrentThreadId(); 
  }

private:
  DWORD id_;
};

/**
 * Wrapper for a win32 thread. We don't use std::thread because it eats exceptions and leads to
 * unusable stack traces.
 */
class ThreadImplWin32 : public Thread {
public:
  ThreadImplWin32(std::function<void()> thread_routine);
  ~ThreadImplWin32();

  /**
   * Join on thread exit.
   */
  void join() override;

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
  ThreadFactoryImplWin32() {}

  ThreadPtr createThread(std::function<void()> thread_routine) override {
    return std::make_unique<ThreadImplWin32>(thread_routine);
  }

  ThreadIdPtr currentThreadId() override { 
    return std::make_unique<ThreadIdImplWin32>(::GetCurrentThreadId());
  }
};

} // namespace Thread
} // namespace Envoy
