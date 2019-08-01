#include <winsock2.h>
#include <mswsock.h>

// <winsock2.h> includes <windows.h>, so undef some interfering symbols
#undef DELETE
#undef GetMessage
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"

#include <io.h>

namespace Envoy {
namespace Api {

SysCallIntResult OsSysCallsImpl::bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) {
  const int rc = ::bind(sockfd, addr, addrlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::connect(SOCKET_FD sockfd, const sockaddr* addr,
                                         socklen_t addrlen) {
  const int rc = ::connect(sockfd, addr, addrlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) {
  const int rc = ::ioctlsocket(sockfd, request, static_cast<u_long*>(argp));
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::writeSocket(SOCKET_FD fd, const void* buffer, size_t num_bytes) {
  const ssize_t rc = ::send(fd, static_cast<const char*>(buffer), num_bytes, 0);
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::writev(SOCKET_FD fd, IOVEC* iovec, int num_iovec) {
  DWORD bytes_sent;
  const int rc = ::WSASend(fd, iovec, num_iovec, &bytes_sent, 0, nullptr, nullptr);
  if (SOCKET_FAILURE(rc)) {
    return {-1, ::WSAGetLastError()};
  }
  return {bytes_sent, 0};
}

SysCallSizeResult OsSysCallsImpl::readv(SOCKET_FD fd, IOVEC* iovec, int num_iovec) {
  DWORD bytes_received;
  DWORD flags = 0;
  const int rc = ::WSARecv(fd, iovec, num_iovec, &bytes_received, &flags, nullptr, nullptr);
  if (SOCKET_FAILURE(rc)) {
    return {-1, ::WSAGetLastError()};
  }
  return {bytes_received, 0};
}

SysCallSizeResult OsSysCallsImpl::recv(SOCKET_FD socket, void* buffer, size_t length, int flags) {
  const ssize_t rc = ::recv(socket, static_cast<char*>(buffer), length, flags);
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::recvfrom(SOCKET_FD sockfd, void* buffer, size_t length, int flags,
                                           struct sockaddr* addr, socklen_t* addrlen) {
  const ssize_t rc = ::recvfrom(sockfd, static_cast<char*>(buffer), length, flags, addr, addrlen);
  return {rc, ::WSAGetLastError()};
}

// TODO Pivotal - copied from https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/netds/winsock/recvmsg/rmmc.cpp
// look into the licensing
LPFN_WSARECVMSG GetWSARecvMsgFunctionPointer()
{
    LPFN_WSARECVMSG     lpfnWSARecvMsg = NULL;
    GUID                guidWSARecvMsg = WSAID_WSARECVMSG;
    SOCKET              sock = INVALID_SOCKET;
    DWORD               dwBytes = 0;

    sock = socket(AF_INET6,SOCK_DGRAM,0);

    if(SOCKET_ERROR == WSAIoctl(sock, 
                                SIO_GET_EXTENSION_FUNCTION_POINTER, 
                                &guidWSARecvMsg, 
                                sizeof(guidWSARecvMsg), 
                                &lpfnWSARecvMsg, 
                                sizeof(lpfnWSARecvMsg), 
                                &dwBytes, 
                                NULL, 
                                NULL
                                ))
    {
        PANIC("WSAIoctl SIO_GET_EXTENSION_FUNCTION_POINTER for WSARecvMsg failed, not implemented?");
        return NULL;
    }

    closesocket(sock);

    return lpfnWSARecvMsg;
}

SysCallSizeResult OsSysCallsImpl::recvmsg(SOCKET_FD sockfd, LPWSAMSG msg, int flags) {
  // msg->dwFlags = flags; TODO Pivotal - Should we implement that?
  static LPFN_WSARECVMSG WSARecvMsg = NULL;
  DWORD bytesRecieved;
  if (NULL == (WSARecvMsg = GetWSARecvMsgFunctionPointer())){
    PANIC("WSARecvMsg has not been implemented by this socket provider");
  }
  // if overlapped and/or comletion routines are supported adjust the arguments accordingly
  const int rc = WSARecvMsg(sockfd, msg, &bytesRecieved, nullptr, nullptr);
  if(rc == SOCKET_ERROR){
    bytesRecieved = -1;
  }
  return {bytesRecieved, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::close(SOCKET_FD fd) {
  const int rc = ::closesocket(fd);
  return {rc, ::WSAGetLastError()};
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

SysCallIntResult OsSysCallsImpl::setsockopt(SOCKET_FD sockfd, int level, int optname,
                                            const void* optval, socklen_t optlen) {
  const int rc = ::setsockopt(sockfd, level, optname, static_cast<const char*>(optval), optlen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                                            socklen_t* optlen) {
  const int rc = ::getsockopt(sockfd, level, optname, static_cast<char*>(optval), optlen);
  return {rc, ::WSAGetLastError()};
}

SysCallSocketResult OsSysCallsImpl::socket(int domain, int type, int protocol) {
  const SOCKET_FD rc = ::socket(domain, type, protocol);
  return {rc, ::WSAGetLastError()};
}

SysCallSizeResult OsSysCallsImpl::sendto(SOCKET_FD fd, const void* buffer, size_t size, int flags,
                                         const sockaddr* addr, socklen_t addrlen) {
  const int rc = ::sendto(fd, static_cast<const char*>(buffer), size, flags, addr, addrlen);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::sendmsg(SOCKET_FD sockfd, const LPWSAMSG msg, int flags) {
  DWORD bytesRecieved;
  // if overlapped and/or comletion routines are supported adjust the arguments accordingly
  const int rc = ::WSASendMsg(sockfd, msg, flags, &bytesRecieved, nullptr, nullptr);
  if(rc == SOCKET_ERROR){
    bytesRecieved = -1;
  }
  return {bytesRecieved, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::getsockname(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) {
  const int rc = ::getsockname(sockfd, name, namelen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::getpeername(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) {
  const int rc = ::getpeername(sockfd, name, namelen);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::setSocketNonBlocking(SOCKET_FD sockfd) {
  u_long iMode = 1;
  const int rc = ::ioctlsocket(sockfd, FIONBIO, &iMode);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::setSocketBlocking(SOCKET_FD sockfd) {
  u_long iMode = 0;
  const int rc = ::ioctlsocket(sockfd, FIONBIO, &iMode);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::shutdown(SOCKET_FD sockfd, int how) {
  const int rc = ::shutdown(sockfd, how);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::listen(SOCKET_FD sockfd, int backlog) {
  const int rc = ::listen(sockfd, backlog);
  return {rc, ::WSAGetLastError()};
}

SysCallIntResult OsSysCallsImpl::socketpair(int domain, int type, int protocol, SOCKET_FD sv[2]) {
  if (sv == nullptr) {
    return {SOCKET_ERROR, WSAEINVAL};
  }

  sv[0] = sv[1] = INVALID_SOCKET;

  SysCallSocketResult socket_result = socket(domain, type, protocol);
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

  socket_result = accept(listener, nullptr, nullptr);
  if (socket_result.rc_ == INVALID_SOCKET) {
    onErr();
    return {SOCKET_ERROR, socket_result.errno_};
  }
  sv[1] = socket_result.rc_;

  ::closesocket(listener);
  return {0, 0};
}

SysCallSocketResult OsSysCallsImpl::accept(SOCKET_FD sockfd, sockaddr* address,
                                           socklen_t* address_len) {
  const SOCKET_FD sock = ::accept(sockfd, address, address_len);
  return {sock, WSAGetLastError()};
}

} // namespace Api
} // namespace Envoy
