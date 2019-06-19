#include "common/network/io_socket_handle_impl.h"

#include <errno.h>

#include <iostream>

#include "envoy/buffer/buffer.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/common/stack_array.h"
#include "common/network/io_socket_error_impl.h"

using Envoy::Api::SysCallIntResult;
using Envoy::Api::SysCallSizeResult;

namespace Envoy {
namespace Network {

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

Api::IoCallUint64Result IoSocketHandleImpl::close() {
#ifdef WIN32
  ASSERT(socket_descriptor_ != INVALID_SOCKET);
  const int rc = ::closesocket(socket_descriptor_);
  socket_descriptor_ = INVALID_SOCKET;
  // TODO: if (rc == SOCKET_ERROR), error should be ::WSAGetLastError()
#else
  ASSERT(fd_ != -1);
  const int rc = ::close(fd_);
  fd_ = -1;
  // TODO: if (rc == -1), error should be errno
#endif
  return Api::IoCallUint64Result(rc, Api::IoErrorPtr(nullptr, IoSocketError::deleteIoError));
}

bool IoSocketHandleImpl::isOpen() const {
#ifdef WIN32
  return socket_descriptor_ != INVALID_SOCKET;
#else
  return fd_ != -1;
#endif
}

Api::IoCallUint64Result IoSocketHandleImpl::readv(uint64_t max_length, Buffer::RawSlice* slices,
                                                  uint64_t num_slice) {
  STACK_ARRAY(iov, IOVEC, num_slice);
  uint64_t num_slices_to_read = 0;
  uint64_t num_bytes_to_read = 0;
  for (; num_slices_to_read < num_slice && num_bytes_to_read < max_length; num_slices_to_read++) {
    IOVEC_SET_BASE(iov[num_slices_to_read], slices[num_slices_to_read].mem_);
    const size_t slice_length = std::min(slices[num_slices_to_read].len_,
                                         static_cast<size_t>(max_length - num_bytes_to_read));
    IOVEC_SET_LEN(iov[num_slices_to_read], slice_length);
    num_bytes_to_read += slice_length;
  }
  ASSERT(num_bytes_to_read <= max_length);
  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  const Api::SysCallSizeResult result =
      os_syscalls.readv(fd(), iov.begin(), static_cast<int>(num_slices_to_read));
  return sysCallResultToIoCallResult(result);
}

Api::IoCallUint64Result IoSocketHandleImpl::writev(const Buffer::RawSlice* slices,
                                                   uint64_t num_slice) {
  STACK_ARRAY(iov, IOVEC, num_slice);
  uint64_t num_slices_to_write = 0;
  for (uint64_t i = 0; i < num_slice; i++) {
    if (slices[i].mem_ != nullptr && slices[i].len_ != 0) {
      IOVEC_SET_BASE(iov[num_slices_to_write], slices[i].mem_);
      IOVEC_SET_LEN(iov[num_slices_to_write], slices[i].len_);
      num_slices_to_write++;
    }
  }
  if (num_slices_to_write == 0) {
    return Api::ioCallUint64ResultNoError();
  }
  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  const Api::SysCallSizeResult result = os_syscalls.writev(fd(), iov.begin(), num_slices_to_write);
  return sysCallResultToIoCallResult(result);
}

Api::IoCallUint64Result
IoSocketHandleImpl::sysCallResultToIoCallResult(const Api::SysCallSizeResult& result) {
  if (result.rc_ >= 0) {
    // Return nullptr as IoError upon success.
    return Api::IoCallUint64Result(result.rc_,
                                   Api::IoErrorPtr(nullptr, IoSocketError::deleteIoError));
  }
  return Api::IoCallUint64Result(
      /*rc=*/0,
      (result.errno_ == EAGAIN
           // EAGAIN is frequent enough that its memory allocation should be avoided.
           ? Api::IoErrorPtr(IoSocketError::getIoSocketEagainInstance(),
                             IoSocketError::deleteIoError)
           : Api::IoErrorPtr(new IoSocketError(result.errno_), IoSocketError::deleteIoError)));
}

} // namespace Network
} // namespace Envoy
