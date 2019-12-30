#pragma once

#include <string>

#include "envoy/api/os_sys_calls.h"

#include "common/singleton/threadsafe_singleton.h"

namespace Envoy {
namespace Api {

class OsSysCallsImpl : public OsSysCalls {
public:
  // Api::OsSysCalls
  SysCallIntResult bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) override;
  SysCallIntResult chmod(const std::string& path, mode_t mode) override;
  SysCallIntResult ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) override;
  SysCallSizeResult writev(SOCKET_FD fd, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult readv(SOCKET_FD fd, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult recv(SOCKET_FD socket, void* buffer, size_t length, int flags) override;
  SysCallSizeResult recvmsg(SOCKET_FD sockfd, LPWSAMSG msg, int flags) override;
  SysCallIntResult close(SOCKET_FD fd) override;
  SysCallIntResult ftruncate(int fd, off_t length) override;
  SysCallPtrResult mmap(void* addr, size_t length, int prot, int flags, int fd,
                        off_t offset) override;
  SysCallIntResult stat(const char* pathname, struct stat* buf) override;
  SysCallIntResult setsockopt(SOCKET_FD sockfd, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                              socklen_t* optlen) override;
  SysCallSocketResult socket(int domain, int type, int protocol) override;
  SysCallSizeResult sendmsg(SOCKET_FD fd, const LPWSAMSG message, int flags) override;
  SysCallIntResult getsockname(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addrlen) override;
  SysCallIntResult gethostname(char* name, size_t length) override;

  // TODO(windows): Exists for porting source/common/network/address_impl.cc
  SysCallIntResult getpeername(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) override;
  SysCallIntResult setsocketblocking(SOCKET_FD sockfd, bool blocking) override;
  SysCallIntResult connect(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) override;
  // TODO(windows): Exists for porting source/common/network/raw_buffer_socket.cc
  SysCallIntResult shutdown(SOCKET_FD sockfd, int how) override;
  // TODO(windows): Exists for porting source/common/filesystem/win32/watcher_impl.cc
  SysCallIntResult socketpair(int domain, int type, int protocol, SOCKET_FD sv[2]) override;
  // TODO(windows): Exists for porting test/test_common/network_utility.cc
  SysCallIntResult listen(SOCKET_FD sockfd, int backlog) override;
  // TODO(windows): Exists for completeness of test/common/api/os_sys_calls_impl_test.cc
  SysCallSizeResult write(SOCKET_FD socket, const void* buffer, size_t length) override;
};

using OsSysCallsSingleton = ThreadSafeSingleton<OsSysCallsImpl>;

} // namespace Api
} // namespace Envoy
