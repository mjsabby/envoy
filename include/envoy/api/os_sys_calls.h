#pragma once

#include <sys/stat.h>

#include <memory>
#include <string>

#include "envoy/api/os_sys_calls_common.h"
#include "envoy/common/platform.h"
#include "envoy/common/pure.h"
#include "envoy/network/io_handle.h"

namespace Envoy {
namespace Api {

class OsSysCalls {
public:
  virtual ~OsSysCalls() = default;

  /**
   * @see bind (man 2 bind)
   */
  virtual SysCallIntResult bind(Network::IoHandle& handle, const sockaddr* addr, socklen_t addrlen) PURE;

  /**
   * @see chmod (man 2 chmod)
   */
  virtual SysCallIntResult chmod(const std::string& path, mode_t mode) PURE;

  /**
   * @see ioctl (man 2 ioctl)
   */
  virtual SysCallIntResult ioctl(Network::IoHandle& handle, unsigned long int request, void* argp) PURE;

  /**
   * @see writev (man 2 writev)
   */
  virtual SysCallSizeResult writev(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) PURE;

  /**
   * @see readv (man 2 readv)
   */
  virtual SysCallSizeResult readv(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) PURE;

  /**
   * @see recv (man 2 recv)
   */
  virtual SysCallSizeResult recv(Network::IoHandle& handle, void* buffer, size_t length, int flags) PURE;

  /**
   * @see recvmsg (man 2 recvmsg)
   */
#ifndef WIN32
  virtual SysCallSizeResult recvmsg(Network::IoHandle& handle, struct msghdr* msg, int flags) PURE;
#else
  virtual SysCallSizeResult recvmsg(Network::IoHandle& handle, WSAMSG* msg, int flags) PURE;
#endif

  /**
   * Release all resources allocated for fd.
   * @return zero on success, -1 returned otherwise.
   */
  virtual SysCallIntResult close(Network::IoHandle& handle) PURE;

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
  virtual SysCallIntResult setsockopt(Network::IoHandle& handle, int level, int optname, const void* optval,
                                      socklen_t optlen) PURE;

  /**
   * @see man 2 getsockopt
   */
  virtual SysCallIntResult getsockopt(Network::IoHandle& handle, int level, int optname, void* optval,
                                      socklen_t* optlen) PURE;

  /**
   * @see man 2 socket
   */
  virtual SysCallIoHandleResult socket(int domain, int type, int protocol) PURE;

  /**
   * @see man 2 sendmsg
   */
#ifdef WIN32
  virtual SysCallSizeResult sendmsg(Network::IoHandle& handle, const LPWSAMSG message, int flags) PURE;
#else
  virtual SysCallSizeResult sendmsg(Network::IoHandle& handle, const msghdr* message, int flags) PURE;
#endif

  /**
   * @see man 2 getsockname
   */
  virtual SysCallIntResult getsockname(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) PURE;

  // TODO(windows): Exists for porting source/common/network/address_impl.cc
  virtual SysCallIntResult getpeername(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) PURE;
  virtual SysCallIntResult setsocketblocking(Network::IoHandle& handle, bool blocking) PURE;
  virtual SysCallIntResult connect(Network::IoHandle& handle, const sockaddr* addr, socklen_t addrlen) PURE;
  // TODO(windows): Exists for porting source/common/network/raw_buffer_socket.cc
  virtual SysCallIntResult shutdown(Network::IoHandle& handle, int how) PURE;
  // TODO(windows): Exists for porting source/common/filesystem/win32/watcher_impl.cc
  virtual SysCallIntResult socketpair(int domain, int type, int protocol, Network::IoHandle* handles[2]) PURE;
  // TODO(windows): Exists for porting test/test_common/network_utility.cc
  virtual SysCallIntResult listen(Network::IoHandle& handle, int backlog) PURE;
  // TODO(windows): Exists for completeness of test/common/api/os_sys_calls_impl_test.cc
  virtual SysCallSizeResult write(Network::IoHandle& handle, const void* buffer, size_t length) PURE;
};

using OsSysCallsPtr = std::unique_ptr<OsSysCalls>;

} // namespace Api
} // namespace Envoy
