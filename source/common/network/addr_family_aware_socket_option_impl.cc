#include "common/network/addr_family_aware_socket_option_impl.h"

#include "envoy/common/exception.h"
#include "envoy/common/platform.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"
#include "common/network/address_impl.h"
#include "common/network/socket_option_impl.h"

namespace Envoy {
namespace Network {

namespace {
Address::IpVersion getVersionFromAddress(Address::InstanceConstSharedPtr addr) {
  if (addr->ip() != nullptr) {
    return addr->ip()->version();
  }
  throw EnvoyException("Unable to set socket option on non-IP sockets");
}

// TODO (YAEL): getVersionFromSocket should probably use the IoHandle in some way
// e.g. just return socket.ioHandle().getVersion() or something to that effect
// Then all this code would live in the appropriate IoHandle
absl::optional<Address::IpVersion> getVersionFromSocket(const Socket& socket) {
#ifdef WIN32
  // getsockname will fail with WSAEINVAL if the socket has not yet been bound or connected
  if (SOCKET_INVALID(socket.ioHandle().fd())) {
    return absl::nullopt;
  }

  WSAPROTOCOL_INFO info;
  int info_size = sizeof(info);
  auto& os_sys_calls = Api::OsSysCallsSingleton::get();
  const Api::SysCallIntResult result =
      os_sys_calls.getsockopt(socket.ioHandle().fd(), SOL_SOCKET, SO_PROTOCOL_INFO,
                              reinterpret_cast<char*>(&info), &info_size);
  if (SOCKET_FAILURE(result.rc_)) {
    return absl::nullopt;
  }

  if (info.iAddressFamily == AF_INET) {
    return absl::optional<Address::IpVersion>(Network::Address::IpVersion::v4);
  } else if (info.iAddressFamily == AF_INET6) {
    return absl::optional<Address::IpVersion>(Network::Address::IpVersion::v6);
  } else {
    return absl::nullopt;
  }
#else
  try {
    // We have local address when the socket is used in a listener but have to
    // infer the IP from the socket FD when initiating connections.
    // TODO(htuch): Figure out a way to obtain a consistent interface for IP
    // version from socket.
    if (socket.localAddress()) {
      return {getVersionFromAddress(socket.localAddress())};
    } else {
      return {getVersionFromAddress(Address::addressFromFd(socket.ioHandle().fd()))};
    }
  } catch (const EnvoyException&) {
    // Ignore, we get here because we failed in getsockname().
    // TODO(htuch): We should probably clean up this logic to avoid relying on exceptions.
  }
  return absl::nullopt;
#endif
}

absl::optional<std::reference_wrapper<SocketOptionImpl>>
getOptionForSocket(const Socket& socket, SocketOptionImpl& ipv4_option,
                   SocketOptionImpl& ipv6_option) {
  auto version = getVersionFromSocket(socket);
  if (!version.has_value()) {
    return absl::nullopt;
  }

  // If the FD is v4, we can only try the IPv4 variant.
  if (*version == Network::Address::IpVersion::v4) {
    return {ipv4_option};
  }
  // If the FD is v6, we first try the IPv6 variant if the platform supports it and fallback to the
  // IPv4 variant otherwise.
  ASSERT(*version == Network::Address::IpVersion::v6);
  if (ipv6_option.isSupported()) {
    return {ipv6_option};
  }
  return {ipv4_option};
}

} // namespace

bool AddrFamilyAwareSocketOptionImpl::setOption(
    Socket& socket, envoy::api::v2::core::SocketOption::SocketState state) const {
  return setIpSocketOption(socket, state, ipv4_option_, ipv6_option_);
}

absl::optional<Socket::Option::Details> AddrFamilyAwareSocketOptionImpl::getOptionDetails(
    const Socket& socket, envoy::api::v2::core::SocketOption::SocketState state) const {
  auto option = getOptionForSocket(socket, *ipv4_option_, *ipv6_option_);

  if (!option.has_value()) {
    return absl::nullopt;
  }

  return option->get().getOptionDetails(socket, state);
}

bool AddrFamilyAwareSocketOptionImpl::setIpSocketOption(
    Socket& socket, envoy::api::v2::core::SocketOption::SocketState state,
    const std::unique_ptr<SocketOptionImpl>& ipv4_option,
    const std::unique_ptr<SocketOptionImpl>& ipv6_option) {
  auto option = getOptionForSocket(socket, *ipv4_option, *ipv6_option);

  if (!option.has_value()) {
    ENVOY_LOG(warn, "Failed to set IP socket option on non-IP socket");
    return false;
  }

  return option->get().setOption(socket, state);
}

} // namespace Network
} // namespace Envoy
