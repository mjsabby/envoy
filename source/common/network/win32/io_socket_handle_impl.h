#pragma once

#include <winsock2.h>

#include "envoy/network/io_handle.h"

// <winsock.h> includes <windows.h>, so undef some interfering symbols. DELETE
// shows up in the base.pb.h header generated from api/envoy/api/core/base.proto.
// Since it's a generated header, we can't just undef the symbol there.
// GetMessage show up in protobuf library code, so again we can't undef the
// symbol there.
#undef DELETE
#undef GetMessage

#include "common/common/assert.h"

namespace Envoy {
namespace Network {

/**
 * IoHandle derivative for Windows sockets
 */
class IoSocketHandleWin32 : public IoHandle {
public:
  IoSocketHandleWin32(SOCKET socket_descriptor_ = INVALID_SOCKET)
      : socket_descriptor_(socket_descriptor_) {}

  // TODO(sbelair2) Call close() in destructor
  ~IoSocketHandleWin32() { ASSERT(socket_descriptor_ == INVALID_SOCKET); }

  // TODO(sbelair2)  To be removed when the fd is fully abstracted from clients.
  SOCKET fd() const override { return socket_descriptor_; }

  // Currently this close() is just for the IoHandle, and the close() system call
  // happens elsewhere. In coming changes, the close() syscall will be made from the IoHandle.
  // In particular, the close should also close the fd.
  void close() override { socket_descriptor_ = INVALID_SOCKET; }

private:
  SOCKET socket_descriptor_;
};
typedef std::unique_ptr<IoSocketHandleWin32> IoSocketHandleWin32Ptr;

} // namespace Network
} // namespace Envoy
