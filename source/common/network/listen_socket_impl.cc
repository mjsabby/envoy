#include "common/network/listen_socket_impl.h"

#if !defined(WIN32)
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include <string>

#include "envoy/common/exception.h"
#include "envoy/common/platform.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/network/address_impl.h"
#include "common/network/utility.h"

namespace Envoy {
namespace Network {

void ListenSocketImpl::doBind() {
  const Api::SysCallIntResult result = local_address_->bind(fd_);
  if (SOCKET_FAILURE(result.rc_)) {
    close();
    throw EnvoyException(
        fmt::format("cannot bind '{}': {}", local_address_->asString(), strerror(result.errno_)));
  }
  if (local_address_->type() == Address::Type::Ip && local_address_->ip()->port() == 0) {
    // If the port we bind is zero, then the OS will pick a free port for us (assuming there are
    // any), and we need to find out the port number that the OS picked.
    local_address_ = Address::addressFromFd(fd_);
  }
}

void ListenSocketImpl::setListenSocketOptions(const Network::Socket::OptionsSharedPtr& options) {
  if (!Network::Socket::applyOptions(options, *this,
                                     envoy::api::v2::core::SocketOption::STATE_PREBIND)) {
    throw EnvoyException("ListenSocket: Setting socket options failed");
  }
}

TcpListenSocket::TcpListenSocket(const Address::InstanceConstSharedPtr& address,
                                 const Network::Socket::OptionsSharedPtr& options,
                                 bool bind_to_port)
    : ListenSocketImpl(address->socket(Address::SocketType::Stream), address) {
  RELEASE_ASSERT(SOCKET_VALID(fd_), "");

  // TODO(htuch): This might benefit from moving to SocketOptionImpl.
  // On Windows, setting SO_REUSEADDR will allow the socket to bind to an address
  // in use by another socket regardless of whether that socket is actively listening.
  // This is in contrast to Linux where the bind will fail if the socket is actively
  // listening but succeed otherwise.
#if !defined(WIN32)
  int on = 1;

  int rc =
      setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&on), sizeof(on));
  RELEASE_ASSERT(rc != -1, "");
#endif

  setListenSocketOptions(options);

  if (bind_to_port) {
    doBind();
  }
}

TcpListenSocket::TcpListenSocket(SOCKET_FD fd, const Address::InstanceConstSharedPtr& address,
                                 const Network::Socket::OptionsSharedPtr& options)
    : ListenSocketImpl(fd, address) {
  setListenSocketOptions(options);
}

#if !defined(WIN32)
UdsListenSocket::UdsListenSocket(const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(address->socket(Address::SocketType::Stream), address) {
  RELEASE_ASSERT(fd_ != -1, "");
  doBind();
}

UdsListenSocket::UdsListenSocket(int fd, const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(fd, address) {}
#endif

} // namespace Network
} // namespace Envoy
