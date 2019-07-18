#pragma once

// TODO(YAEL) As the IoHandle work progresses, all the SOCKET_FD and releated macros (such as IOVEC)
// in all files should disappear

#ifndef WIN32

#include <sys/ioctl.h>
#include <sys/mman.h>   // for mode_t
#include <sys/socket.h> // for sockaddr
#include <sys/uio.h>    // for iovec

#else
// TODO(YAEL) - PR this changes, if needed, after IoHandle is done.
// (this was already PR'd with comment in other files
// https://github.com/envoyproxy/envoy/blob/b09a6d4a33137c5547fb6aec03bdaa07214b260f/source/common/filesystem/win32/directory_iterator_impl.h#L6-L11)
// notice that this change should be PR'd in other files as well

#include <ws2tcpip.h>
// <ws2tcpip.h> includes <windows.h>, so undef some interfering symbols
#undef DELETE
#undef GetMessage
#endif

#include <sys/stat.h>

#include <memory>
#include <string>

#include "envoy/api/os_sys_calls_common.h"
#include "envoy/common/pure.h"

namespace Envoy {
namespace Api {

class OsSysCalls {
public:
  virtual ~OsSysCalls() = default;

  /**
   * @see bind (man 2 bind)
   */
  virtual SysCallIntResult bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * @see ioctl (man 2 ioctl)
   */
  virtual SysCallIntResult ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) PURE;

  /**
   * @see writev (man 2 writev)
   */
  virtual SysCallSizeResult writev(SOCKET_FD fd, IOVEC* iovec, int num_iovec) PURE;

  /**
   * @see readv (man 2 readv)
   */
  virtual SysCallSizeResult readv(SOCKET_FD fd, IOVEC* iovec, int num_iovec) PURE;

  /**
   * @see recv (man 2 recv)
   */
  virtual SysCallSizeResult recv(SOCKET_FD socket, void* buffer, size_t length, int flags) PURE;

  /**
   * @see recv (man 2 recvfrom)
   */
  virtual SysCallSizeResult recvfrom(SOCKET_FD sockfd, void* buffer, size_t length, int flags,
                                     struct sockaddr* addr, socklen_t* addrlen) PURE;

  /**
   * @see recvmsg (man 2 recvmsg)
   */
  #ifndef WIN32
  virtual SysCallSizeResult recvmsg(SOCKET_FD sockfd, struct msghdr* msg, int flags) PURE;
  #else
  virtual SysCallSizeResult recvmsg(SOCKET_FD sockfd, WSAMSG* msg, int flags) PURE;
  #endif

  /**
   * Release all resources allocated for fd.
   * @return zero on success, -1 returned otherwise.
   */
  virtual SysCallIntResult close(SOCKET_FD fd) PURE;

  /**
   * @see man 2 ftruncate
   */
  virtual SysCallIntResult ftruncate(int fd, off_t length) PURE;

  /**
   * @see man 2 mmap
   */
  virtual SysCallPtrResult mmap(void* addr, size_t length, int prot, int flags, int fd,
                                off_t offset) PURE;

  /**
   * @see man 2 stat
   */
  virtual SysCallIntResult stat(const char* pathname, struct stat* buf) PURE;

  /**
   * @see man 2 setsockopt
   */
  virtual SysCallIntResult setsockopt(SOCKET_FD sockfd, int level, int optname, const void* optval,
                                      socklen_t optlen) PURE;

  /**
   * @see man 2 getsockopt
   */
  virtual SysCallIntResult getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                                      socklen_t* optlen) PURE;

  /**
   * @see man 2 socket
   */
  virtual SysCallSocketResult socket(int domain, int type, int protocol) PURE;

  /**
   * @see man 2 sendto
   */
  virtual SysCallSizeResult sendto(SOCKET_FD sockfd, const void* buffer, size_t size, int flags,
                                   const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * @see man 2 sendmsg
   */
#ifdef WIN32
  virtual SysCallSizeResult sendmsg(SOCKET_FD sockfd, const LPWSAMSG message, int flags) PURE;
#else
  virtual SysCallSizeResult sendmsg(SOCKET_FD sockfd, const msghdr* message, int flags) PURE;
#endif

  /**
   * @see man 2 getsockname
   */
  virtual SysCallIntResult getsockname(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) PURE;

  //TODO: Pivotal review - the following functions don't exist in master
  /**
   * @see man 2 getpeername
   */
  virtual SysCallIntResult getpeername(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) PURE;

  virtual SysCallIntResult setSocketNonBlocking(SOCKET_FD sockfd) PURE;

  virtual SysCallIntResult setSocketBlocking(SOCKET_FD sockfd) PURE;

  /**
   * @see man 2 shutdown
   */
  virtual SysCallIntResult shutdown(SOCKET_FD sockfd, int how) PURE;

  /**
   * @see man 2 listen
   */
  virtual SysCallIntResult listen(SOCKET_FD sockfd, int backlog) PURE;

  /**
   * @see man 2 socketpair
   */
  virtual SysCallIntResult socketpair(int domain, int type, int protocol, SOCKET_FD sv[2]) PURE;

  /**
   * @see man 2 accept
   */
  virtual SysCallSocketResult accept(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addr_len) PURE;


  /*
   * @see connect (man 2 connect)
   */
  virtual SysCallIntResult connect(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * Write num_bytes to fd from buffer.
   * @return number of bytes written if non negative, otherwise error code.
   */
  virtual SysCallSizeResult writeSocket(SOCKET_FD fd, const void* buffer, size_t num_bytes) PURE;

};

using OsSysCallsPtr = std::unique_ptr<OsSysCalls>;

} // namespace Api
} // namespace Envoy
