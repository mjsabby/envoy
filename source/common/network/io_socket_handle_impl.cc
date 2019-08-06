#include "common/network/io_socket_handle_impl.h"

#include <cerrno>
#include <iostream>

#ifdef WIN32
#include "winsock2.h"
// winsock2.h includes <windows.h>, so undef some interfering symbols
#undef DELETE
#undef GetMessage
#endif

#include "envoy/buffer/buffer.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/common/stack_array.h"
#include "common/network/address_impl.h"
#include "common/network/io_socket_error_impl.h"

#include "absl/types/optional.h"

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
  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  const auto& result = os_syscalls.close(fd_);
  const int rc = result.rc_;
  fd_ = -1;
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

Api::IoCallUint64Result IoSocketHandleImpl::sendto(const Buffer::RawSlice& slice, int flags,
                                                   const Address::Instance& address) {
  const auto* address_base = dynamic_cast<const Address::InstanceBase*>(&address);
  sockaddr* sock_addr = const_cast<sockaddr*>(address_base->sockAddr());

  auto& os_syscalls = Api::OsSysCallsSingleton::get();
#ifdef WIN32
  const Api::SysCallSizeResult result = os_syscalls.sendto(socket_descriptor_, slice.mem_, slice.len_, flags,
                                                           sock_addr, address_base->sockAddrLen());
#else
  const Api::SysCallSizeResult result = os_syscalls.sendto(fd_, slice.mem_, slice.len_, flags,
                                                           sock_addr, address_base->sockAddrLen());
#endif
  return sysCallResultToIoCallResult(result);
}

Api::IoCallUint64Result IoSocketHandleImpl::sendmsg(const Buffer::RawSlice* slices,
                                                    uint64_t num_slice, int flags,
                                                    const Address::Ip* self_ip,
                                                    const Address::Instance& peer_address) {
  const auto* address_base = dynamic_cast<const Address::InstanceBase*>(&peer_address);
  sockaddr* sock_addr = const_cast<sockaddr*>(address_base->sockAddr());

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
  Api::SysCallSizeResult result;
#ifdef WIN32
  WSAMSG message;
  message.name = reinterpret_cast<LPSOCKADDR>(sock_addr);
  message.namelen = address_base->sockAddrLen();
  message.lpBuffers = iov.begin();
  message.dwBufferCount = num_slices_to_write;
  message.Control.buf = nullptr;
  message.Control.len = 0;
  message.dwFlags = 0;

  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  result = os_syscalls.sendmsg(socket_descriptor_, &message, flags);
#else
  struct msghdr message;
  message.msg_name = reinterpret_cast<void*>(sock_addr);
  message.msg_namelen = address_base->sockAddrLen();
  message.msg_iov = iov.begin();
  message.msg_iovlen = num_slices_to_write;
  message.msg_flags = 0;
  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  if (self_ip == nullptr) {
    message.msg_control = nullptr;
    message.msg_controllen = 0;
    result = os_syscalls.sendmsg(fd_, &message, flags);
  } else {
    const size_t space_v6 = CMSG_SPACE(sizeof(in6_pktinfo));
    // FreeBSD only needs in_addr size, but allocates more to unify code in two platforms.
    const size_t space_v4 = CMSG_SPACE(sizeof(in_pktinfo));
    const size_t cmsg_space = (space_v4 < space_v6) ? space_v6 : space_v4;
    // kSpaceForIp should be big enough to hold both IPv4 and IPv6 packet info.
    STACK_ARRAY(cbuf, char, cmsg_space);
    memset(cbuf.begin(), 0, cmsg_space);

    message.msg_control = cbuf.begin();
    message.msg_controllen = cmsg_space * sizeof(char);
    cmsghdr* const cmsg = CMSG_FIRSTHDR(&message);
    RELEASE_ASSERT(cmsg != nullptr, fmt::format("cbuf with size {} is not enough, cmsghdr size {}",
                                                sizeof(cbuf), sizeof(cmsghdr)));
    if (self_ip->version() == Address::IpVersion::v4) {
      cmsg->cmsg_level = IPPROTO_IP;
#ifndef IP_SENDSRCADDR
      cmsg->cmsg_len = CMSG_LEN(sizeof(in_pktinfo));
      cmsg->cmsg_type = IP_PKTINFO;
      auto pktinfo = reinterpret_cast<in_pktinfo*>(CMSG_DATA(cmsg));
      pktinfo->ipi_ifindex = 0;
      pktinfo->ipi_spec_dst.s_addr = self_ip->ipv4()->address();
#else
      cmsg->cmsg_type = IP_SENDSRCADDR;
      cmsg->cmsg_len = CMSG_LEN(sizeof(in_addr));
      *(reinterpret_cast<struct in_addr*>(CMSG_DATA(cmsg))).s_addr = self_ip->ipv4()->address();
#endif
    } else if (self_ip->version() == Address::IpVersion::v6) {
      cmsg->cmsg_len = CMSG_LEN(sizeof(in6_pktinfo));
      cmsg->cmsg_level = IPPROTO_IPV6;
      cmsg->cmsg_type = IPV6_PKTINFO;
      auto pktinfo = reinterpret_cast<in6_pktinfo*>(CMSG_DATA(cmsg));
      pktinfo->ipi6_ifindex = 0;
      *(reinterpret_cast<absl::uint128*>(pktinfo->ipi6_addr.s6_addr)) = self_ip->ipv6()->address();
    }
    result = os_syscalls.sendmsg(fd_, &message, flags);
  }
#endif
  return sysCallResultToIoCallResult(result);
}

Api::IoCallUint64Result IoSocketHandleImpl::sysCallResultToIoCallResult(const Api::SysCallSizeResult& result) {
  if (result.rc_ >= 0) {
    // Return nullptr as IoError upon success.
    return Api::IoCallUint64Result(result.rc_,
                                   Api::IoErrorPtr(nullptr, IoSocketError::deleteIoError));
  }
  RELEASE_ASSERT(result.errno_ != EINVAL, "Invalid argument passed in.");
  return Api::IoCallUint64Result(
      /*rc=*/0,
      (result.errno_ == EAGAIN
           // EAGAIN is frequent enough that its memory allocation should be avoided.
           ? Api::IoErrorPtr(IoSocketError::getIoSocketEagainInstance(),
                             IoSocketError::deleteIoError)
           : Api::IoErrorPtr(new IoSocketError(result.errno_), IoSocketError::deleteIoError)));
}

#ifndef WIN32
Address::InstanceConstSharedPtr maybeGetDstAddressFromHeader(const struct cmsghdr& cmsg,
                                                             uint32_t self_port) {
#else
Address::InstanceConstSharedPtr maybeGetDstAddressFromHeader(const WSACMSGHDR& cmsg,
                                                             uint32_t self_port) {
#endif  
  if (cmsg.cmsg_type == IPV6_PKTINFO) {
    auto info = reinterpret_cast<const in6_pktinfo*>(CMSG_DATA(&cmsg));
    sockaddr_storage ss;
    auto ipv6_addr = reinterpret_cast<sockaddr_in6*>(&ss);
    memset(ipv6_addr, 0, sizeof(sockaddr_in6));
    ipv6_addr->sin6_family = AF_INET6;
    ipv6_addr->sin6_addr = info->ipi6_addr;
    ipv6_addr->sin6_port = htons(self_port);
    return Address::addressFromSockAddr(ss, sizeof(sockaddr_in6), /*v6only=*/false);
  }
#ifndef IP_RECVDSTADDR
  if (cmsg.cmsg_type == IP_PKTINFO) {
    auto info = reinterpret_cast<const in_pktinfo*>(CMSG_DATA(&cmsg));
#else
  if (cmsg.cmsg_type == IP_RECVDSTADDR) {
    auto addr = reinterpret_cast<const in_addr*>(CMSG_DATA(&cmsg));
#endif
    sockaddr_storage ss;
    auto ipv4_addr = reinterpret_cast<sockaddr_in*>(&ss);
    memset(ipv4_addr, 0, sizeof(sockaddr_in));
    ipv4_addr->sin_family = AF_INET;
    ipv4_addr->sin_addr =
#ifndef IP_RECVDSTADDR
        info->ipi_addr;
#else
        *addr;
#endif
    ipv4_addr->sin_port = htons(self_port);
    return Address::addressFromSockAddr(ss, sizeof(sockaddr_in), /*v6only=*/false);
  }
  return nullptr;
}

absl::optional<uint32_t> maybeGetPacketsDroppedFromHeader(
#ifdef SO_RXQ_OVFL
    const struct cmsghdr& cmsg) {
  if (cmsg.cmsg_type == SO_RXQ_OVFL) {
    return *reinterpret_cast<const uint32_t*>(CMSG_DATA(&cmsg));
  }
#else
    const struct cmsghdr&) {
#endif
  return absl::nullopt;
}

Api::IoCallUint64Result IoSocketHandleImpl::recvmsg(Buffer::RawSlice* slices,
                                                    const uint64_t num_slice, uint32_t self_port,
                                                    RecvMsgOutput& output) {

  // The minimum cmsg buffer size to filled in destination address and packets dropped when
  // receiving a packet. It is possible for a received packet to contain both IPv4 and IPv6
  // addresses.
  const size_t cmsg_space = CMSG_SPACE(sizeof(int)) + CMSG_SPACE(sizeof(struct in_pktinfo)) +
                            CMSG_SPACE(sizeof(struct in6_pktinfo));
  STACK_ARRAY(cbuf, char, cmsg_space);
  memset(cbuf.begin(), 0, cmsg_space);

  STACK_ARRAY(iov, IOVEC, num_slice);
  uint64_t num_slices_for_read = 0;
  for (uint64_t i = 0; i < num_slice; i++) {
    if (slices[i].mem_ != nullptr && slices[i].len_ != 0) {
      IOVEC_SET_BASE(iov[num_slices_for_read], slices[i].mem_);
      IOVEC_SET_LEN(iov[num_slices_for_read], slices[i].len_);
      num_slices_for_read++;
    }
  }
  if (num_slices_for_read == 0) {
    return Api::ioCallUint64ResultNoError();
  }
#ifdef WIN32
  SOCKADDR_STORAGE peer_addr;
  WSAMSG hdr;
  hdr.name = reinterpret_cast<LPSOCKADDR>(&peer_addr);
  hdr.namelen = sizeof(peer_addr);
  hdr.lpBuffers = iov.begin();
  hdr.dwBufferCount = num_slices_for_read;
  hdr.dwFlags = 0;
  hdr.Control.buf = reinterpret_cast<CHAR*>(&cbuf);
  hdr.Control.len = cmsg_space;

  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  const Api::SysCallSizeResult result = os_syscalls.recvmsg(socket_descriptor_, &hdr, 0);
#else
  sockaddr_storage peer_addr;
  msghdr hdr;
  hdr.msg_name = &peer_addr;
  hdr.msg_namelen = sizeof(peer_addr);
  hdr.msg_iov = iov.begin();
  hdr.msg_iovlen = num_slices_for_read;
  hdr.msg_flags = 0;

  auto control = reinterpret_cast<struct cmsghdr*>(cbuf.begin());
  control->cmsg_len = cmsg_space;
  hdr.msg_control = control;
  hdr.msg_controllen = cmsg_space;
  auto& os_sys_calls = Api::OsSysCallsSingleton::get();
  const Api::SysCallSizeResult result = os_sys_calls.recvmsg(fd_, &hdr, 0);
#endif
  if (result.rc_ < 0) {
    return sysCallResultToIoCallResult(result);
  }

#ifdef WIN32
  RELEASE_ASSERT((hdr.dwFlags & MSG_CTRUNC) == 0,
                 fmt::format("Incorrectly set control message length: {}", hdr.Control.len));
  RELEASE_ASSERT(hdr.namelen > 0,
                 fmt::format("Unable to get remote address from recvmsg() for fd: {}", socket_descriptor_));
#else
  RELEASE_ASSERT((hdr.msg_flags & MSG_CTRUNC) == 0,
                 fmt::format("Incorrectly set control message length: {}", hdr.msg_controllen));
  RELEASE_ASSERT(hdr.msg_namelen > 0,
                 fmt::format("Unable to get remote address from recvmsg() for fd: {}", fd_));
#endif
  try {
    // Set v6only to false so that mapped-v6 address can be normalize to v4
    // address. Though dual stack may be disabled, it's still okay to assume the
    // address is from a dual stack socket. This is because mapped-v6 address
    // must come from a dual stack socket. An actual v6 address can come from
    // both dual stack socket and v6 only socket. If |peer_addr| is an actual v6
    // address and the socket is actually v6 only, the returned address will be
    // regarded as a v6 address from dual stack socket. However, this address is not going to be
    // used to create socket. Wrong knowledge of dual stack support won't hurt.
#ifdef WIN32
    output.peer_address_ =
        Address::addressFromSockAddr(peer_addr, hdr.namelen, /*v6only=*/false);
#else
    output.peer_address_ =
        Address::addressFromSockAddr(peer_addr, hdr.msg_namelen, /*v6only=*/false);
#endif
  } catch (const EnvoyException& e) {
    PANIC(fmt::format("Invalid remote address for fd: {}, error: {}", fd(), e.what()));
  }

  // Get overflow, local and peer addresses from control message.
#ifdef WIN32
  LPWSACMSGHDR cmsg;
#else
  struct cmsghdr* cmsg;
#endif
  for (cmsg = CMSG_FIRSTHDR(&hdr); cmsg != nullptr; cmsg = CMSG_NXTHDR(&hdr, cmsg)) {
    if (output.local_address_ == nullptr) {
      try {
        Address::InstanceConstSharedPtr addr = maybeGetDstAddressFromHeader(*cmsg, self_port);
        if (addr != nullptr) {
          // This is a IP packet info message.
          output.local_address_ = std::move(addr);
          continue;
        }
      } catch (const EnvoyException& e) {
        PANIC(fmt::format("Invalid destination address for fd: {}, error: {}", fd(), e.what()));
      }
    }
    if (output.dropped_packets_ != nullptr) {
      absl::optional<uint32_t> maybe_dropped = maybeGetPacketsDroppedFromHeader(*cmsg);
      if (maybe_dropped) {
        *output.dropped_packets_ = *maybe_dropped;
      }
    }
  }
  return sysCallResultToIoCallResult(result);
}

} // namespace Network
} // namespace Envoy