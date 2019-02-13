#pragma once

#include "envoy/api/os_sys_calls.h"
#include "envoy/network/io_handle.h"

namespace Envoy {
namespace Network {

class IoSocketError : public Api::IoError {
public:
  explicit IoSocketError(int sys_errno) : errno_(sys_errno) {}

  ~IoSocketError() override {}

private:
  IoErrorCode errorCode() const override;

  std::string errorDetails() const override;

  int errno_;
};

/**
 * IoHandle derivative for sockets
 */
class IoSocketHandleImpl : public IoHandle {
public:
#ifdef WIN32
  explicit IoSocketHandleImpl(SOCKET socket_descriptor = INVALID_SOCKET)
      : socket_descriptor_(socket_descriptor) {}
#else
  explicit IoSocketHandleImpl(int fd = -1) : fd_(fd) {}
#endif
  // Close underlying socket if close() hasn't been call yet.
  ~IoSocketHandleImpl() override;

  // TODO(sbelair2)  To be removed when the fd is fully abstracted from clients.
#ifdef WIN32
  SOCKET fd() const override { return socket_descriptor_; }
#else
  int fd() const override { return fd_; }
#endif

  Api::IoCallUintResult close() override;

  bool isOpen() const override;

private:
#ifdef WIN32
  SOCKET socket_descriptor_;
#else
  int fd_;
#endif
};

} // namespace Network
} // namespace Envoy
