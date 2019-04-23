#pragma once

#include "envoy/api/os_sys_calls.h"

#include "common/singleton/threadsafe_singleton.h"

namespace Envoy {
namespace Api {

class OsSysCallsImpl : public OsSysCalls {
public:
  // Api::OsSysCalls
  SysCallIntResult bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) override;
  SysCallIntResult connect(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) override;
  SysCallIntResult ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) override;
  SysCallSizeResult writeSocket(SOCKET_FD fd, const void* buffer, size_t num_bytes) override;
  SysCallSizeResult writev(SOCKET_FD fd, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult readv(SOCKET_FD fd, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult recv(SOCKET_FD socket, void* buffer, size_t length, int flags) override;
  SysCallSizeResult recvfrom(SOCKET_FD sockfd, void* buffer, size_t length, int flags,
                             struct sockaddr* addr, socklen_t* addrlen) override;
  SysCallIntResult close(SOCKET_FD fd) override;
  SysCallIntResult shmOpen(const char* name, int oflag, mode_t mode) override;
  SysCallIntResult shmUnlink(const char* name) override;
  SysCallIntResult ftruncate(int fd, off_t length) override;
  SysCallPtrResult mmap(void* addr, size_t length, int prot, int flags, int fd,
                        off_t offset) override;
  SysCallIntResult stat(const char* pathname, struct stat* buf) override;
  SysCallIntResult setsockopt(SOCKET_FD sockfd, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                              socklen_t* optlen) override;
  SysCallSocketResult socket(int domain, int type, int protocol) override;
  SysCallIntResult getsockname(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) override;
  SysCallIntResult getpeername(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) override;
  SysCallIntResult setSocketNonBlocking(SOCKET_FD sockfd) override;
  SysCallIntResult setSocketBlocking(SOCKET_FD sockfd) override;
  SysCallIntResult shutdown(SOCKET_FD sockfd, int how) override;
  SysCallIntResult listen(SOCKET_FD sockfd, int backlog) override;
  SysCallIntResult socketpair(int domain, int type, int protocol, SOCKET_FD sv[2]) override;
  SysCallSocketResult accept(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addr_len) override;
  // TODO(YAEL) Check if we need the sched function for windows, and if not, whether we can define it only for linux and not in the base class
  SysCallIntResult sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask) override;
};

typedef ThreadSafeSingleton<OsSysCallsImpl> OsSysCallsSingleton;

} // namespace Api
} // namespace Envoy
