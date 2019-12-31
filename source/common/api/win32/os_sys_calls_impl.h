#pragma once

#include <string>

#include "envoy/api/os_sys_calls.h"

#include "common/singleton/threadsafe_singleton.h"

namespace Envoy {
namespace Api {

class OsSysCallsImpl : public OsSysCalls {
public:
  // Api::OsSysCalls
  SysCallIntResult bind(Network::IoHandle& handle, const sockaddr* addr, socklen_t addrlen) override;
  SysCallIntResult chmod(const std::string& path, mode_t mode) override;
  SysCallIntResult ioctl(Network::IoHandle& handle, unsigned long int request, void* argp) override;
  SysCallSizeResult writev(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult readv(Network::IoHandle& handle, IOVEC* iovec, int num_iovec) override;
  SysCallSizeResult recv(Network::IoHandle& handle, void* buffer, size_t length, int flags) override;
  SysCallSizeResult recvmsg(Network::IoHandle& handle, LPWSAMSG msg, int flags) override;
  SysCallIntResult close(Network::IoHandle& handle) override;
  SysCallIntResult ftruncate(int fd, off_t length) override;
  SysCallPtrResult mmap(void* addr, size_t length, int prot, int flags, int fd,
                        off_t offset) override;
  SysCallIntResult stat(const char* pathname, struct stat* buf) override;
  SysCallIntResult setsockopt(Network::IoHandle& handle, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(Network::IoHandle& handle, int level, int optname, void* optval,
                              socklen_t* optlen) override;
  SysCallIoHandleResult socket(int domain, int type, int protocol) override;
  SysCallSizeResult sendmsg(Network::IoHandle& handle, const LPWSAMSG message, int flags) override;
  SysCallIntResult getsockname(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) override;

  // TODO(windows): Exists for porting source/common/network/address_impl.cc
  SysCallIntResult getpeername(Network::IoHandle& handle, sockaddr* name, socklen_t* namelen) override;
  SysCallIntResult setsocketblocking(Network::IoHandle& handle, bool blocking) override;
  SysCallIntResult connect(Network::IoHandle& handle, const sockaddr* addr, socklen_t addrlen) override;
  // TODO(windows): Exists for porting source/common/network/raw_buffer_socket.cc
  SysCallIntResult shutdown(Network::IoHandle& handle, int how) override;
  // TODO(windows): Exists for porting source/common/filesystem/win32/watcher_impl.cc
  SysCallIntResult socketpair(int domain, int type, int protocol, Network::IoHandle* handles[2]) override;
  // TODO(windows): Exists for porting test/test_common/network_utility.cc
  SysCallIntResult listen(Network::IoHandle& handle, int backlog) override;
  // TODO(windows): Exists for completeness of test/common/api/os_sys_calls_impl_test.cc
  SysCallSizeResult write(Network::IoHandle& handle, const void* buffer, size_t length) override;
};

using OsSysCallsSingleton = ThreadSafeSingleton<OsSysCallsImpl>;

} // namespace Api
} // namespace Envoy
