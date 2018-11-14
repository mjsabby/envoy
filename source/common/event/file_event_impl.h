#pragma once

#include <cstdint>

#include "envoy/common/platform.h"
#include "envoy/event/file_event.h"

#include "common/event/dispatcher_impl.h"
#include "common/event/event_impl_base.h"

namespace Envoy {
namespace Event {

/**
 * Implementation of FileEvent for libevent that uses persistent events and
 * assumes the user will read/write until EAGAIN is returned from the file.
 */
class StreamEventImpl : public StreamEvent {
public:
  StreamEventImpl(DispatcherImpl& dispatcher, SOCKET_FD fd, OnReadCb rcb, OnWriteCb wcb);
  ~StreamEventImpl();

  // Event::FileEvent
  //void activate(uint32_t events) override;
  //void setEnabled(uint32_t events) override;
  uv_buf_t* buf_;

private:
  //void assignEvents(uint32_t events);
  int startRead(uv_buf_t *buf);
  int stopRead();
  int write(uv_write_t* req, const uv_buf_t* bufs, unsigned int nbufs);

  OnReadCb rcb_;
  OnWriteCb wcb_;
  uv_loop_t* loop_;
  SOCKET_FD fd_;
  uv_any_handle raw_handle_;
  uv_shutdown_t shutdown_req_;
};

} // namespace Event
} // namespace Envoy
