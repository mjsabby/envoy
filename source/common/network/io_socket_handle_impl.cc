#include "common/network/io_socket_handle_impl.h"

#include <errno.h>

#include <iostream>

#include "common/common/assert.h"

using Envoy::Api::SysCallIntResult;
using Envoy::Api::SysCallSizeResult;

namespace Envoy {
namespace Network {

Api::IoError::IoErrorCode IoSocketError::errorCode() const {
#ifdef WIN32
  switch (errno_) {
  case WSAEWOULDBLOCK:
    // WSAEWOULDBLOCK should use specific error ENVOY_ERROR_AGAIN.
    NOT_REACHED_GCOVR_EXCL_LINE;
  case WSAEOPNOTSUPP:
    return IoErrorCode::NoSupport;
  case WSAEAFNOSUPPORT:
    return IoErrorCode::AddressFamilyNoSupport;
  case WSAEINPROGRESS:
    return IoErrorCode::InProgress;
  case WSAEACCES:
    return IoErrorCode::Permission;
  default:
    return IoErrorCode::UnknownError;
  }
#else
  switch (errno_) {
  case EAGAIN:
    // EAGAIN should use specific error ENVOY_ERROR_AGAIN.
    NOT_REACHED_GCOVR_EXCL_LINE;
  case ENOTSUP:
    return IoErrorCode::NoSupport;
  case EAFNOSUPPORT:
    return IoErrorCode::AddressFamilyNoSupport;
  case EINPROGRESS:
    return IoErrorCode::InProgress;
  case EPERM:
    return IoErrorCode::Permission;
  default:
    return IoErrorCode::UnknownError;
  }
#endif
}

std::string IoSocketError::errorDetails() const {
#ifdef WIN32
  DWORD flags =
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

  char* buffer;

  DWORD rc =
      ::FormatMessage(flags, nullptr, errno_, 0, reinterpret_cast<LPTSTR>(&buffer), 0, nullptr);
  ASSERT(rc != 0);
  std::string ret(buffer);
  ::LocalFree(buffer);
  return ret;
#else
  return ::strerror(errno_);
#endif
}

using IoSocketErrorPtr = std::unique_ptr<IoSocketError, Api::IoErrorDeleterType>;

// Deallocate memory only if the error is not ENVOY_ERROR_AGAIN.
void deleteIoError(Api::IoError* err) {
  ASSERT(err != nullptr);
  if (err != ENVOY_ERROR_AGAIN) {
    delete err;
  }
}

template <typename T> Api::IoCallResult<T> resultFailure(T result, int sys_errno) {
  return {result, IoSocketErrorPtr(new IoSocketError(sys_errno), deleteIoError)};
}

template <typename T> Api::IoCallResult<T> resultSuccess(T result) {
  return {result, IoSocketErrorPtr(nullptr, [](Api::IoError*) { NOT_REACHED_GCOVR_EXCL_LINE; })};
}

IoSocketHandleImpl::~IoSocketHandleImpl() {
#ifdef WIN32
  if (socket_descriptor_ != INVALID_SOCKET) {
    IoSocketHandleImpl::close();
  }
#else
  if (fd_ != -1) {
    IoSocketHandleImpl::close();
  }
#endif
}

Api::IoCallUintResult IoSocketHandleImpl::close() {
#ifdef WIN32
  ASSERT(socket_descriptor_ != INVALID_SOCKET);
  const int rc = ::closesocket(socket_descriptor_);
  socket_descriptor_ = INVALID_SOCKET;
  return rc != SOCKET_ERROR ? resultSuccess<uint64_t>(rc)
                            : resultFailure<uint64_t>(rc, ::WSAGetLastError());
#else
  ASSERT(fd_ != -1);
  const int rc = ::close(fd_);
  fd_ = -1;
  return rc != -1 ? resultSuccess<uint64_t>(rc) : resultFailure<uint64_t>(rc, errno);
#endif
}

bool IoSocketHandleImpl::isOpen() const {
#ifdef WIN32
  return socket_descriptor_ != INVALID_SOCKET;
#else
  return fd_ != -1;
#endif
}

} // namespace Network
} // namespace Envoy
