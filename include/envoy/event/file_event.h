#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "envoy/common/pure.h"

#include "uv.h"

namespace Envoy {
namespace Event {

//struct FileReadyType {
//  // File is ready for reading.
//  static const uint32_t Read = 0x1;
//  // File is ready for writing.
//  static const uint32_t Write = 0x2;
//  // File has been remote closed.
//  static const uint32_t Closed = 0x4;
//};

//enum class FileTriggerType { Level, Edge };

/**
 * Callback invoked when a FileEvent is ready for reading or writing.
 */
typedef std::function<void(uv_stream_t *s, ssize_t nread, const uv_buf_t * buf)> OnReadCb;
typedef std::function<void(uv_write_t *w, int status)> OnWriteCb;

/**
 * Wrapper for file based (read/write) event notifications.
 */
class StreamEvent {
public:
  virtual ~StreamEvent() {}

  /**
   * Activate the file event explicitly for a set of events. Should be a logical OR of FileReadyType
   * events. This method "injects" the event (and fires callbacks) regardless of whether the event
   * is actually ready on the underlying file.
   */
  //virtual void activate(uint32_t events) PURE;

  ///**
  // * Enable the file event explicitly for a set of events. Should be a logical OR of FileReadyType
  // * events. As opposed to activate(), this routine causes the file event to listen for the
  // * registered events and fire callbacks when they are active.
  // */
  //virtual void setEnabled(uint32_t events) PURE;
  virtual int startRead(uv_buf_t *buf) PURE;
  virtual int stopRead() PURE;
  virtual int write(uv_write_t* req, const uv_buf_t* bufs, unsigned int nbufs) PURE;
};

typedef std::unique_ptr<StreamEvent> StreamEventPtr;

} // namespace Event
} // namespace Envoy
