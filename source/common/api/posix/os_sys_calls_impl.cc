#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <string>

#include "common/api/os_sys_calls_impl.h"

namespace Envoy {
namespace Api {

SysCallIntResult OsSysCallsImpl::bind(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen) {
  const int rc = ::bind(sockfd, addr, addrlen);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::chmod(const std::string& path, mode_t mode) {
  const int rc = ::chmod(path.c_str(), mode);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::ioctl(SOCKET_FD sockfd, unsigned long int request, void* argp) {
  const int rc = ::ioctl(sockfd, request, argp);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::close(SOCKET_FD fd) {
  const int rc = ::close(fd);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::writev(SOCKET_FD fd, IOVEC* iovec, int num_iovec) {
  const ssize_t rc = ::writev(fd, iovec, num_iovec);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::readv(SOCKET_FD fd, IOVEC* iovec, int num_iovec) {
  const ssize_t rc = ::readv(fd, iovec, num_iovec);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::recv(SOCKET_FD socket, void* buffer, size_t length, int flags) {
  const ssize_t rc = ::recv(socket, buffer, length, flags);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::recvmsg(int sockfd, struct msghdr* msg, int flags) {
  const ssize_t rc = ::recvmsg(sockfd, msg, flags);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::ftruncate(int fd, off_t length) {
  const int rc = ::ftruncate(fd, length);
  return {rc, errno};
}

SysCallPtrResult OsSysCallsImpl::mmap(void* addr, size_t length, int prot, int flags, int fd,
                                      off_t offset) {
  void* rc = ::mmap(addr, length, prot, flags, fd, offset);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::stat(const char* pathname, struct stat* buf) {
  const int rc = ::stat(pathname, buf);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::setsockopt(SOCKET_FD sockfd, int level, int optname,
                                            const void* optval, socklen_t optlen) {
  const int rc = ::setsockopt(sockfd, level, optname, optval, optlen);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                                            socklen_t* optlen) {
  const int rc = ::getsockopt(sockfd, level, optname, optval, optlen);
  return {rc, errno};
}

SysCallSocketResult OsSysCallsImpl::socket(int domain, int type, int protocol) {
  const SOCKET_FD rc = ::socket(domain, type, protocol);
  return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::sendmsg(int fd, const msghdr* message, int flags) {
  const int rc = ::sendmsg(fd, message, flags);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::getsockname(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addrlen) {
  const int rc = ::getsockname(sockfd, addr, addrlen);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::gethostname(char* name, size_t length) {
  const int rc = ::gethostname(name, length);
  return {rc, errno};
}

// TODO(windows): added for porting source/common/network/address_impl.cc
SysCallIntResult OsSysCallsImpl::getpeername(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen) {
  const int rc = ::getpeername(sockfd, name, namelen);
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::setsocketblocking(SOCKET_FD sockfd, bool blocking) {
  const int flags = ::fcntl(sockfd, F_GETFL, 0);
  int rc;
  if (flags == -1) {
    return {-1, errno};
  }
  if (blocking) {
    rc = ::fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
  } else {
    rc = ::fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
  }
  return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::connect(SOCKET_FD sockfd, const sockaddr* addr,
                                         socklen_t addrlen) {
  const int rc = ::connect(sockfd, addr, addrlen);
  return {rc, errno};
}

// TODO(windows): added for porting source/common/network/raw_buffer_socket.cc
SysCallIntResult OsSysCallsImpl::shutdown(SOCKET_FD sockfd, int how) {
  const int rc = ::shutdown(sockfd, how);
  return {rc, errno};
}

// TODO(windows): added for porting source/common/filesystem/win32/watcher_impl.cc
SysCallIntResult OsSysCallsImpl::socketpair(int domain, int type, int protocol, SOCKET_FD sv[2]) {
  const int rc = ::socketpair(domain, type, protocol, sv);
  return {rc, errno};
}

// TODO(windows): added for porting test/test_common/network_utility.cc
SysCallIntResult OsSysCallsImpl::listen(SOCKET_FD sockfd, int backlog) {
  const int rc = ::listen(sockfd, backlog);
  return {rc, errno};
}

// TODO(windows): added for completeness testing test/common/api/os_sys_calls_impl_test.cc
SysCallSizeResult OsSysCallsImpl::write(SOCKET_FD sockfd, const void* buffer, size_t length) {
  const ssize_t rc = ::write(sockfd, buffer, length);
  return {rc, errno};
}

} // namespace Api
} // namespace Envoy
