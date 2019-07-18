#pragma once

#include <memory>
#include <string>

#include "envoy/api/api.h"
#include "envoy/api/os_sys_calls.h"
#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"

#include "common/api/os_sys_calls_impl.h"
#if defined(__linux__)
#include "common/api/os_sys_calls_impl_linux.h"
#endif
#include "test/mocks/filesystem/mocks.h"
#include "test/mocks/stats/mocks.h"
#include "test/test_common/test_time.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Api {

class MockApi : public Api {
public:
  MockApi();
  ~MockApi() override;

  // Api::Api
  Event::DispatcherPtr allocateDispatcher() override;
  Event::DispatcherPtr allocateDispatcher(Buffer::WatermarkFactoryPtr&& watermark_factory) override;
  TimeSource& timeSource() override { return time_system_; }

  MOCK_METHOD1(allocateDispatcher_, Event::Dispatcher*(Event::TimeSystem&));
  MOCK_METHOD2(allocateDispatcher_,
               Event::Dispatcher*(Buffer::WatermarkFactoryPtr&& watermark_factory,
                                  Event::TimeSystem&));
  MOCK_METHOD0(fileSystem, Filesystem::Instance&());
  MOCK_METHOD0(threadFactory, Thread::ThreadFactory&());
  MOCK_METHOD0(rootScope, const Stats::Scope&());

  testing::NiceMock<Filesystem::MockInstance> file_system_;
  Event::GlobalTimeSystem time_system_;
  testing::NiceMock<Stats::MockIsolatedStatsStore> stats_store_;
};

class MockOsSysCalls : public OsSysCallsImpl {
public:
  MockOsSysCalls();
  ~MockOsSysCalls() override;

  // Api::OsSysCalls
  SysCallIntResult setsockopt(SOCKET_FD sockfd, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                              socklen_t* optlen) override;

  MOCK_METHOD3(bind, SysCallIntResult(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen));
  MOCK_METHOD3(ioctl, SysCallIntResult(SOCKET_FD sockfd, unsigned long int request, void* argp));
  MOCK_METHOD1(close, SysCallIntResult(SOCKET_FD fd));
  MOCK_METHOD3(writev, SysCallSizeResult(SOCKET_FD, IOVEC*, int));
  MOCK_METHOD3(sendmsg, SysCallSizeResult(SOCKET_FD fd, const msghdr* message, int flags));
  MOCK_METHOD3(readv, SysCallSizeResult(SOCKET_FD, IOVEC*, int));
  MOCK_METHOD4(recv, SysCallSizeResult(SOCKET_FD socket, void* buffer, size_t length, int flags));
  MOCK_METHOD6(recvfrom, SysCallSizeResult(SOCKET_FD sockfd, void* buffer, size_t length, int flags,
                                           struct sockaddr* addr, socklen_t* addrlen));
  MOCK_METHOD3(recvmsg, SysCallSizeResult(int socket, struct msghdr* msg, int flags));
  MOCK_METHOD2(ftruncate, SysCallIntResult(int fd, off_t length));
  MOCK_METHOD6(mmap, SysCallPtrResult(void* addr, size_t length, int prot, int flags, int fd,
                                      off_t offset));
  MOCK_METHOD2(stat, SysCallIntResult(const char* name, struct stat* stat));
  MOCK_METHOD5(setsockopt_,
               int(SOCKET_FD sockfd, int level, int optname, const void* optval, socklen_t optlen));
  MOCK_METHOD5(getsockopt_,
               int(SOCKET_FD sockfd, int level, int optname, void* optval, socklen_t* optlen));
  MOCK_METHOD3(socket, SysCallSocketResult(int domain, int type, int protocol));

  // TODO: Pivotal Review - These functions do not exist in master
  MOCK_METHOD3(getsockname, SysCallIntResult(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen));
  MOCK_METHOD3(getpeername, SysCallIntResult(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen));
  MOCK_METHOD1(setSocketNonBlocking, SysCallIntResult(SOCKET_FD sockfd));
  MOCK_METHOD1(setSocketBlocking, SysCallIntResult(SOCKET_FD sockfd));
  MOCK_METHOD2(shutdown, SysCallIntResult(SOCKET_FD sockfd, int how));
  MOCK_METHOD2(listen, SysCallIntResult(SOCKET_FD sockfd, int backlog));
  MOCK_METHOD4(socketpair, SysCallIntResult(int domain, int type, int protocol, SOCKET_FD sv[2]));
  MOCK_METHOD3(accept, SysCallSocketResult(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addr_len));
  MOCK_METHOD3(connect,
               SysCallIntResult(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen));
  MOCK_METHOD3(writeSocket, SysCallSizeResult(SOCKET_FD, const void*, size_t));
  // end TODO

  // Map from (sockfd,level,optname) to boolean socket option.
  using SockOptKey = std::tuple<SOCKET_FD, int, int>;
  std::map<SockOptKey, bool> boolsockopts_;
};

#if defined(__linux__)
class MockLinuxOsSysCalls : public LinuxOsSysCallsImpl {
public:
  // Api::LinuxOsSysCalls
  MOCK_METHOD3(sched_getaffinity, SysCallIntResult(pid_t pid, size_t cpusetsize, cpu_set_t* mask));
};
#endif

} // namespace Api
} // namespace Envoy
