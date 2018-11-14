#include "common/event/libevent.h"

#include <signal.h>

#include "common/common/assert.h"

#include "uv.h"

void uv_loop_deleter(uv_loop_t * l) {
  const int rc = uv_loop_close(l);
  ASSERT(rc == 0);
}

namespace Envoy {
namespace Event {
namespace Libevent {

bool Global::initialized_ = false;

void Global::initialize() {
//#if !defined(WIN32)
//  evthread_use_pthreads();
//
//  // Ignore SIGPIPE and allow errors to propagate through error codes.
//  signal(SIGPIPE, SIG_IGN);
//#else
//  evthread_use_windows_threads();
//#endif
  initialized_ = true;
}

} // namespace Libevent
} // namespace Event
} // namespace Envoy
