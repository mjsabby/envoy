#include "common/event/libevent.h"

#include <signal.h>

#include "common/common/assert.h"

#include "event2/thread.h"

namespace Envoy {
namespace Event {
namespace Libevent {

bool Global::initialized_ = false;

void Global::initialize() {
// TODO(YAEL) - PR story #164317693
#ifdef WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();

  // Ignore SIGPIPE and allow errors to propagate through error codes.
  signal(SIGPIPE, SIG_IGN);
#endif
  initialized_ = true;
}

} // namespace Libevent
} // namespace Event
} // namespace Envoy
