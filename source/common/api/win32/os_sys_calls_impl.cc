#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include <string>

#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"
#include "common/network/io_socket_handle_impl.h"

namespace Envoy {
namespace Api {

SysCallIntResult OsSysCallsImpl::bind(Network::IoHandle& handle, const sockaddr* addr, socklen_t addrlen) {
  const int rc = ::bind(handle.fd(), addr, addrlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::chmod(const std::string& path, mode_t mode) {
  const int rc = ::_chmod(path.c_str(), mode);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::ioctl(Network::IoHandle& handle, unsigned long int request, void* argp) {
  const int rc = ::ioctlsocket(handle.fd(), request, static_cast<u_long*>(argp));
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::close(Network::IoHandle& handle) {
  const int rc = ::closesocket(handle.fd());
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::writev(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) {
  DWORD bytes_sent;
  const int rc = ::WSASend(handle.fd(), iovec, num_iovec, &bytes_sent, 0, nullptr, nullptr);
  if (SOCKET_FAILURE(rc)) {
    return {-1, ::WSAGetLastError()};
  }
  return {bytes_sent, 0};
}

SysCallSizeResult OsSysCallsImpl::readv(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) {
  DWORD bytes_received;
  DWORD flags = 0;
  const int rc = ::WSARecv(handle.fd(), iovec, num_iovec, &bytes_received, &flags, nullptr, nullptr);
  if (SOCKET_FAILURE(rc)) {
    return {-1, ::WSAGetLastError()};
  }
  return {bytes_received, 0};
}

SysCallSizeResult OsSysCallsImpl::recv(Network::IoHandle& handle, void* buffer, size_t length, int flags) {
  const ssize_t rc = ::recv(handle.fd(), static_cast<char*>(buffer), length, flags);
  return {rc, ::WSAGetLastError()};
}

// TODO Pivotal - copied from
// https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/netds/winsock/recvmsg/rmmc.cpp
// look into the licensing
LPFN_WSARECVMSG GetWSARecvMsgFunctionPointer() {
  LPFN_WSARECVMSG lpfnWSARecvMsg = NULL;
  GUID guidWSARecvMsg = WSAID_WSARECVMSG;
  SOCKET sock = INVALID_SOCKET;
  DWORD dwBytes = 0;

  sock = socket(AF_INET6, SOCK_DGRAM, 0);

  if (SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidWSARecvMsg,
                               sizeof(guidWSARecvMsg), &lpfnWSARecvMsg, sizeof(lpfnWSARecvMsg),
                               &dwBytes, NULL, NULL)) {
    PANIC("WSAIoctl SIO_GET_EXTENSION_FUNCTION_POINTER for WSARecvMsg failed, not implemented?");
    return NULL;
  }

  closesocket(sock);

  return lpfnWSARecvMsg;
}

SysCallSizeResult OsSysCallsImpl::recvmsg(Network::IoHandle& handle, LPWSAMSG msg, int flags) {
  // msg->dwFlags = flags; TODO Pivotal - Should we implement that?
  static LPFN_WSARECVMSG WSARecvMsg = NULL;
  DWORD bytesRecieved;
  if (NULL == (WSARecvMsg = GetWSARecvMsgFunctionPointer())) {
    PANIC("WSARecvMsg has not been implemented by this socket provider");
  }
  // if overlapped and/or completion routines are supported adjust the arguments accordingly
  const int rc = WSARecvMsg(handle.fd(), msg, &bytesRecieved, nullptr, nullptr);
  if (rc == SOCKET_ERROR) {
    bytesRecieved = -1;
  }
  return {bytesRecieved, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::ftruncate(int fd, off_t length) {
  const int rc = ::_chsize_s(fd, length);
  return {rc, errno};
}

SysCallPtrResult OsSysCallsImpl::mmap(void* addr, size_t length, int prot, int flags, int fd,
                                      off_t offset) {
  PANIC("mmap not implemented on Windows");
}

SysCallIntResult OsSysCallsImpl::stat(const char* pathname, struct stat* buf) {
  const int rc = ::stat(pathname, buf);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::setsockopt(Network::IoHandle& handle, int level, int optname,
                                            const void* optval, socklen_t optlen) {
  const int rc = ::setsockopt(handle.fd(), level, optname, static_cast<const char*>(optval), optlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::getsockopt(Network::IoHandle& handle, int level, int optname, void* optval,
                                            socklen_t* optlen) {
  const int rc = ::getsockopt(handle.fd(), level, optname, static_cast<char*>(optval), optlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIoHandleResult OsSysCallsImpl::socket(int domain, int type, int protocol) {
  const SOCKET socket = ::socket(domain, type, protocol);
  Network::IoSocketHandleImpl rc_(socket);
  return {rc_, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::sendmsg(Network::IoHandle& handle, const LPWSAMSG msg, int flags) {
  DWORD bytesRecieved;
  // if overlapped and/or completion routines are supported adjust the arguments accordingly
  const int rc = ::WSASendMsg(handle.fd(), msg, flags, &bytesRecieved, nullptr, nullptr);
  if (rc == SOCKET_ERROR) {
    bytesRecieved = -1;
  }
  return {bytesRecieved, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::getsockname(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) {
  const int rc = ::getsockname(handle.fd(), name, namelen);
  return {rc, ::WSAGetLastError()};
}

// TODO(windows): added for porting source/common/network/address_impl.cc
SysCallIntResult OsSysCallsImpl::getpeername(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) {
  const int rc = ::getpeername(handle.fd(), name, namelen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::setsocketblocking(Network::IoHandle& handle, bool blocking) {
  u_long iMode = blocking ? 0 : 1;
  const int rc = ::ioctlsocket(handle.fd(), FIONBIO, &iMode);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::connect(Network::IoHandle& handle, const sockaddr* addr,
                                         socklen_t addrlen) {
  const int rc = ::connect(handle.fd(), addr, addrlen);
  return {rc, ::WSAGetLastError()};
}

// TODO(windows): added for porting source/common/network/raw_buffer_socket.cc
SysCallIntResult OsSysCallsImpl::shutdown(Network::IoHandle& handle, int how) {
  const int rc = ::shutdown(handle.fd(), how);
  return {rc, ::WSAGetLastError()};
}

// TODO(windows): added for porting source/common/filesystem/win32/watcher_impl.cc
SysCallIntResult OsSysCallsImpl::socketpair(int domain, int type, int protocol, Network::IoHandle *handles[2]) {
  if (handles == nullptr) {
    return {SOCKET_ERROR, WSAEINVAL};
  }

  sv[0] = sv[1] = INVALID_SOCKET;

  SysCallIoHandleResult socket_result = socket(domain, type, protocol);
  if (socket_result.rc_ == INVALID_SOCKET) {
    return {SOCKET_ERROR, socket_result.errno_};
  }

  SOCKET_FD listener = socket_result.rc_;

  typedef union {
    struct sockaddr_storage sa;
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
  } sa_union;
  sa_union a = {};
  socklen_t sa_size = sizeof(a);

  a.sa.ss_family = domain;
  if (domain == AF_INET) {
    a.in.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
    a.in.sin_port = 0;
  } else if (domain == AF_INET6) {
    a.in6.sin6_addr = in6addr_loopback;
    a.in6.sin6_port = 0;
  } else {
    return {SOCKET_ERROR, WSAEINVAL};
  }

  auto onErr = [this, listener, sv]() -> void {
    ::closesocket(listener);
    ::closesocket(sv[0]);
    ::closesocket(sv[1]);
    sv[0] = INVALID_SOCKET;
    sv[1] = INVALID_SOCKET;
  };

  SysCallIntResult int_result = bind(listener, reinterpret_cast<sockaddr*>(&a), sa_size);
  if (int_result.rc_ == SOCKET_ERROR) {
    onErr();
    return int_result;
  }

  int_result = listen(listener, 1);
  if (int_result.rc_ == SOCKET_ERROR) {
    onErr();
    return int_result;
  }

  socket_result = socket(domain, type, protocol);
  if (socket_result.rc_ == INVALID_SOCKET) {
    onErr();
    return {SOCKET_ERROR, socket_result.errno_};
  }
  sv[0] = socket_result.rc_;

  a = {};
  int_result = getsockname(listener, reinterpret_cast<sockaddr*>(&a), &sa_size);
  if (int_result.rc_ == SOCKET_ERROR) {
    onErr();
    return int_result;
  }

  int_result = connect(sv[0], reinterpret_cast<sockaddr*>(&a), sa_size);
  if (int_result.rc_ == SOCKET_ERROR) {
    onErr();
    return int_result;
  }

  socket_result.rc_ = ::accept(listener, nullptr, nullptr);
  if (socket_result.rc_ == INVALID_SOCKET) {
    socket_result.errno_ = ::WSAGetLastError();
    onErr();
    return {SOCKET_ERROR, socket_result.errno_};
  }
  sv[1] = socket_result.rc_;

  ::closesocket(listener);
  return {0, 0};
}

// TODO(windows): added for porting test/test_common/network_utility.cc
SysCallIntResult OsSysCallsImpl::listen(Network::IoHandle& handle, int backlog) {
  const int rc = ::listen(handle.fd(), backlog);
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::write(Network::IoHandle& handle, const void* buffer, size_t length) {
  const ssize_t rc = ::send(handle.fd(), static_cast<const char*>(buffer), length, 0);
  return {rc, ::WSAGetLastError()};
}

} // namespace Api
} // namespace Envoy
