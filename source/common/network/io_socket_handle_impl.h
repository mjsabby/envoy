#pragma once

#include "envoy/api/io_error.h"
#include "envoy/api/os_sys_calls.h"
#include "envoy/network/io_handle.h"

#include "common/common/logger.h"

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
class IoSocketHandleImpl : public IoHandle, protected Logger::Loggable<Logger::Id::io> {
public:
  explicit IoSocketHandleImpl(int fd = -1) : fd_(fd) {}

  // Close underlying socket if close() hasn't been call yet.
  ~IoSocketHandleImpl() override;

  // TODO(sbelair2)  To be removed when the fd is fully abstracted from clients.
  int fd() const override { return fd_; }

  Api::IoCallUint64Result close() override;

  bool isOpen() const override;

  Api::IoCallUint64Result readv(uint64_t max_length, Buffer::RawSlice* slices,
                                uint64_t num_slice) override;

  Api::IoCallUint64Result writev(const Buffer::RawSlice* slices, uint64_t num_slice) override;

  Api::IoCallUint64Result sendto(const Buffer::RawSlice& slice, int flags,
                                 const Address::Instance& address) override;

  Api::IoCallUint64Result sendmsg(const Buffer::RawSlice* slices, uint64_t num_slice, int flags,
                                  const Address::Instance& address) override;

  Api::IoCallUint64Result recvmsg(Buffer::RawSlice* slices, const uint64_t num_slice,
                                  uint32_t self_port, RecvMsgOutput& output) override;

private:
  // Converts a SysCallSizeResult to IoCallUint64Result.
  Api::IoCallUint64Result sysCallResultToIoCallResult(const Api::SysCallSizeResult& result);

  int fd_;
};

} // namespace Network
} // namespace Envoy
