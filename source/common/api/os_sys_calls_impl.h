#pragma once

#include <string>

#include "envoy/api/os_sys_calls.h"

#include "common/singleton/threadsafe_singleton.h"

namespace Envoy {
namespace Api {

class OsSysCallsImpl : public OsSysCalls {
public:
  // Api::OsSysCalls
  SysCallIntResult bind(int64_t sockfd, const sockaddr* addr, socklen_t addrlen) override;
  SysCallIntResult chmod(const std::string& path, mode_t mode) override;
  SysCallIntResult ioctl(int64_t sockfd, unsigned long int request, void* argp) override;
  SysCallSizeResult writev(int64_t fd, const iovec* iovec, int num_iovec) override;
  SysCallSizeResult readv(int64_t fd, const iovec* iovec, int num_iovec) override;
  SysCallSizeResult recv(int64_t socket, void* buffer, size_t length, int flags) override;
  SysCallSizeResult recvmsg(int64_t sockfd, struct msghdr* msg, int flags) override;
  SysCallIntResult close(int64_t fd) override;
  SysCallIntResult ftruncate(int64_t fd, off_t length) override;
  SysCallPtrResult mmap(void* addr, size_t length, int prot, int flags, int64_t fd,
                        off_t offset) override;
  SysCallIntResult stat(const char* pathname, struct stat* buf) override;
  SysCallIntResult setsockopt(int64_t sockfd, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(int64_t sockfd, int level, int optname, void* optval,
                              socklen_t* optlen) override;
  SysCallIntResult socket(int domain, int type, int protocol) override;
  SysCallSizeResult sendmsg(int64_t fd, const msghdr* message, int flags) override;
  SysCallIntResult getsockname(int64_t sockfd, sockaddr* addr, socklen_t* addrlen) override;
};

using OsSysCallsSingleton = ThreadSafeSingleton<OsSysCallsImpl>;

} // namespace Api
} // namespace Envoy
