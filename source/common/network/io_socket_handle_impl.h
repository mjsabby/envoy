#pragma once

#include "envoy/api/io_error.h"
#include "envoy/api/os_sys_calls.h"
#include "envoy/network/io_handle.h"

#include "common/common/logger.h"

namespace Envoy {
namespace Network {

/**
 * IoHandle derivative for sockets
 */
class IoSocketHandleImpl : public IoHandle, protected Logger::Loggable<Logger::Id::io> {
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

  Api::IoCallUint64Result close() override;

  bool isOpen() const override;

  Api::IoCallUint64Result readv(uint64_t max_length, Buffer::RawSlice* slices,
                                uint64_t num_slice) override;

  Api::IoCallUint64Result writev(const Buffer::RawSlice* slices, uint64_t num_slice) override;

  Api::IoCallUint64Result sendto(const Buffer::RawSlice& slice, int flags,
                                 const Address::Instance& address) override;

  Api::IoCallUint64Result sendmsg(const Buffer::RawSlice* slices, uint64_t num_slice, int flags,
                                  const Address::Ip* self_ip,
                                  const Address::Instance& peer_address) override;

  Api::IoCallUint64Result recvmsg(Buffer::RawSlice* slices, const uint64_t num_slice,
                                  uint32_t self_port, RecvMsgOutput& output) override;

private:
#ifdef WIN32
  SOCKET socket_descriptor_;
#else
  // Converts a SysCallSizeResult to IoCallUint64Result.
  Api::IoCallUint64Result sysCallResultToIoCallResult(const Api::SysCallSizeResult& result);

  int fd_;
#endif
};

} // namespace Network
} // namespace Envoy
