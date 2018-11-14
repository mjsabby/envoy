#pragma once

#include "common/common/c_smart_ptr.h"

#include "uv.h"


void uv_loop_deleter(uv_loop_t * l);

//extern "C" {
//void uv_loop_close(uv_loop_t*);
//}

//struct evbuffer;
//extern "C" {
//void evbuffer_free(evbuffer*);
//}
//
//struct bufferevent;
//extern "C" {
//void bufferevent_free(bufferevent*);
//}
//
//struct evconnlistener;
//extern "C" {
//void evconnlistener_free(evconnlistener*);
//}

namespace Envoy {
namespace Event {
namespace Libevent {

/**
 * Global functionality specific to libevent.
 */
class Global {
public:
  static bool initialized() { return initialized_; }

  /**
   * Initialize the library globally.
   */
  static void initialize();

private:
  // True if initialized() has been called.
  static bool initialized_;
};

typedef CSmartPtr<uv_loop_t, uv_loop_deleter> LoopPtr;
//typedef CSmartPtr<evbuffer, evbuffer_free> BufferPtr;
//typedef CSmartPtr<bufferevent, bufferevent_free> BufferEventPtr;
//typedef CSmartPtr<evconnlistener, evconnlistener_free> ListenerPtr;

} // namespace Libevent
} // namespace Event
} // namespace Envoy
