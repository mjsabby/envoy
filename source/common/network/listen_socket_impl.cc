#include "common/network/listen_socket_impl.h"

#ifndef WIN32
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include <string>

#include "envoy/common/exception.h"
#include "envoy/common/platform.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/network/address_impl.h"
#include "common/network/utility.h"

namespace Envoy {
namespace Network {

void ListenSocketImpl::doBind() {
  const Api::SysCallIntResult result = local_address_->bind(io_handle_->fd());
  if (SOCKET_FAILURE(result.rc_)) {
    close();
    throw SocketBindException(
        fmt::format("cannot bind '{}': {}", local_address_->asString(), strerror(result.errno_)),
        result.errno_);
  }
  if (local_address_->type() == Address::Type::Ip && local_address_->ip()->port() == 0) {
    // If the port we bind is zero, then the OS will pick a free port for us (assuming there are
    // any), and we need to find out the port number that the OS picked.
    local_address_ = Address::addressFromFd(io_handle_->fd());
  }
}

void ListenSocketImpl::setListenSocketOptions(const Network::Socket::OptionsSharedPtr& options) {
  if (!Network::Socket::applyOptions(options, *this,
                                     envoy::api::v2::core::SocketOption::STATE_PREBIND)) {
    throw EnvoyException("ListenSocket: Setting socket options failed");
  }
}

void ListenSocketImpl::setupSocket(const Network::Socket::OptionsSharedPtr& options,
                                   bool bind_to_port) {
  setListenSocketOptions(options);

  if (bind_to_port) {
    doBind();
  }
}

template <>
void NetworkListenSocket<
    NetworkSocketTrait<Address::SocketType::Stream>>::setPrebindSocketOptions() {

  // On Windows, setting SO_REUSEADDR will allow the socket to bind to an address
  // in use by another socket regardless of whether that socket is actively listening.
  // This is in contrast to Linux where the bind will fail if the socket is actively
  // listening but succeed otherwise.
#ifndef WIN32
  int on = 1;
  auto& os_syscalls = Api::OsSysCallsSingleton::get();
  Api::SysCallIntResult status =
      os_syscalls.setsockopt(io_handle_->fd(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  RELEASE_ASSERT(status.rc_ != -1, "failed to set SO_REUSEADDR socket option");
#endif
}

template <>
void NetworkListenSocket<
    NetworkSocketTrait<Address::SocketType::Datagram>>::setPrebindSocketOptions() {}

// No such thing as AF_UNIX on Windows
#ifndef WIN32
UdsListenSocket::UdsListenSocket(const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(address->socket(Address::SocketType::Stream), address) {
  RELEASE_ASSERT(io_handle_->fd() != -1, "");
  doBind();
}

UdsListenSocket::UdsListenSocket(IoHandlePtr&& io_handle,
                                 const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(std::move(io_handle), address) {}
#endif

} // namespace Network
} // namespace Envoy
