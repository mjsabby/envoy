#include "common/event/file_event_impl.h"

#include <cstdint>

#include "common/common/assert.h"
#include "common/event/dispatcher_impl.h"

#include "uv.h"

namespace Envoy {
namespace Event {

StreamEventImpl::StreamEventImpl(DispatcherImpl& dispatcher, SOCKET_FD fd, OnReadCb rcb,
                             OnWriteCb wcb)
    : rcb_(rcb), wcb_(wcb), loop_(&dispatcher.loop()), fd_(fd) {
//#if defined(WIN32)
//  RELEASE_ASSERT(trigger_ == FileTriggerType::Level,
//                 "libevent does not support edge triggers on Windows");
//#endif
//  assignEvents(events);
//  event_add(&raw_event_, nullptr);

  // assume that FD is a tcp socket
  int rc = uv_tcp_init(loop_, &raw_handle_.tcp);
  ASSERT(rc == 0);

  rc = uv_tcp_open(&raw_handle_.tcp, fd);
  ASSERT(rc == 0);

  raw_handle_.stream.data = this;
}

StreamEventImpl::~StreamEventImpl() {
  // Derived classes are assumed to have already assigned the raw event in the constructor.
  printf("destroying stream\n");
  int rc = uv_read_stop(&raw_handle_.stream);
  ASSERT(rc == 0);

  auto closeCb = [](uv_handle_t *) {
    printf("closed!\n");
  }; 

  uv_close(&raw_handle_.handle, closeCb);
}


//void FileEventImpl::activate(uint32_t events) {
//  int libevent_events = 0;
//  if (events & FileReadyType::Read) {
//    libevent_events |= EV_READ;
//  }
//
//  if (events & FileReadyType::Write) {
//    libevent_events |= EV_WRITE;
//  }
//
//  if (events & FileReadyType::Closed) {
//    libevent_events |= EV_CLOSED;
//  }
//
//  ASSERT(libevent_events);
//  event_active(&raw_event_, libevent_events, 0);
//}
//
//void FileEventImpl::assignEvents(uint32_t events) {
//  event_assign(&raw_event_, base_, fd_,
//               EV_PERSIST | (trigger_ == FileTriggerType::Level ? 0 : EV_ET) |
//                   (events & FileReadyType::Read ? EV_READ : 0) |
//                   (events & FileReadyType::Write ? EV_WRITE : 0) |
//                   (events & FileReadyType::Closed ? EV_CLOSED : 0),
//               [](evutil_socket_t, short what, void* arg) -> void {
//                 FileEventImpl* event = static_cast<FileEventImpl*>(arg);
//                 uint32_t events = 0;
//                 if (what & EV_READ) {
//                   events |= FileReadyType::Read;
//                 }
//
//                 if (what & EV_WRITE) {
//                   events |= FileReadyType::Write;
//                 }
//
//                 if (what & EV_CLOSED) {
//                   events |= FileReadyType::Closed;
//                 }
//
//                 ASSERT(events);
//                 event->cb_(events);
//               },
//               this);
//}
//
//void FileEventImpl::setEnabled(uint32_t events) {
//  event_del(&raw_event_);
//  assignEvents(events);
//  event_add(&raw_event_, nullptr);
//}
int StreamEventImpl::startRead(uv_buf_t *buf) {
  buf_ = buf;

  auto allocCb = [](uv_handle_t* handle, size_t, uv_buf_t* buf) {
    printf("hey, an alloc callback\n");
    StreamEventImpl* event = static_cast<StreamEventImpl*>(handle->data);
    buf->base = event->buf_->base;
    buf->len = event->buf_->len;
  };

  auto readCb = [](uv_stream_t *s, ssize_t nread, const uv_buf_t * buf) {
    printf("hey, an read callback\n");
    StreamEventImpl* event = static_cast<StreamEventImpl*>(s->data);
    event->rcb_(s, nread, buf);
  };

  return uv_read_start(&raw_handle_.stream, allocCb, readCb);
}

int StreamEventImpl::stopRead() {
  return uv_read_stop(&raw_handle_.stream);
}

int StreamEventImpl::write(uv_write_t* req, const uv_buf_t* bufs, unsigned int nbufs) {
  auto writeCb = [](uv_write_t *w, int status) {
    printf("hey, an write callback\n");
    StreamEventImpl* event = static_cast<StreamEventImpl*>(w->handle->data);
    event->wcb_(w, status);
  };

  return uv_write(req, &raw_handle_.stream, bufs, nbufs, writeCb);
}

} // namespace Event
} // namespace Envoy
