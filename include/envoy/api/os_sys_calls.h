#pragma once

#if !defined(WIN32)
#include <sys/ioctl.h>
#include <sys/mman.h>   // for mode_t
#include <sys/socket.h> // for sockaddr
#include <sys/uio.h>    // for iovec

#else
#include <ws2tcpip.h>
// <ws2tcpip.h> includes <windows.h>, so undef some interfering symbols
#undef DELETE
#undef GetMessage
#endif

#include <sys/stat.h>

#include <memory>
#include <string>

#include "envoy/common/pure.h"
#include "envoy/common/platform.h"

namespace Envoy {
namespace Api {

/**
 * SysCallResult holds the rc and errno values resulting from a system call.
 */
template <typename T> struct SysCallResult {

  /**
   * The return code from the system call.
   */
  T rc_;

  /**
   * The errno value as captured after the system call.
   */
  int errno_;
};

typedef SysCallResult<int> SysCallIntResult;
typedef SysCallResult<ssize_t> SysCallSizeResult;
typedef SysCallResult<void*> SysCallPtrResult;
typedef SysCallResult<std::string> SysCallStringResult;
typedef SysCallResult<bool> SysCallBoolResult;
typedef SysCallResult<SOCKET_FD> SysCallSocketResult;

class OsSysCalls {
public:
  virtual ~OsSysCalls() {}

  /**
   * @see bind (man 2 bind)
   */
  virtual SysCallIntResult bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * @see connect (man 2 connect)
   */
  virtual SysCallIntResult connect(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * @see ioctl (man 2 ioctl)
   */
  virtual SysCallIntResult ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) PURE;

  /**
   * Write num_bytes to fd from buffer.
   * @return number of bytes written if non negative, otherwise error code.
   */
  virtual SysCallSizeResult writeSocket(SOCKET_FD fd, const void* buffer, size_t num_bytes) PURE;

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
   * Release all resources allocated for fd.
   * @return zero on success, -1 returned otherwise.
   */
  virtual SysCallIntResult closeSocket(SOCKET_FD fd) PURE;

  /**
   * @see shm_open (man 3 shm_open)
   */
  virtual SysCallIntResult shmOpen(const char* name, int oflag, mode_t mode) PURE;

  /**
   * @see shm_unlink (man 3 shm_unlink)
   */
  virtual SysCallIntResult shmUnlink(const char* name) PURE;

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
   * @see man 2 getsockname
   */
  virtual SysCallIntResult getsockname(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) PURE;

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
};

typedef std::unique_ptr<OsSysCalls> OsSysCallsPtr;

} // namespace Api
} // namespace Envoy
