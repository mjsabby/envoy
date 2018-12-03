#pragma once

#include <memory>
#include <string>

#include "envoy/api/api.h"
#include "envoy/api/os_sys_calls.h"
#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"
#include "envoy/stats/store.h"

#include "common/api/os_sys_calls_impl.h"

#include "test/mocks/filesystem/mocks.h"
#include "test/test_common/test_time_system.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Api {

class MockApi : public Api {
public:
  MockApi();
  ~MockApi();

  // Api::Api
  Event::DispatcherPtr allocateDispatcher(Event::TimeSystem& time_system) override {
    return Event::DispatcherPtr{allocateDispatcher_(time_system)};
  }

  MOCK_METHOD1(allocateDispatcher_, Event::Dispatcher*(Event::TimeSystem&));
  MOCK_METHOD3(createFile,
               Filesystem::FileSharedPtr(const std::string& path, Event::Dispatcher& dispatcher,
                                         Thread::BasicLockable& lock));
  MOCK_METHOD1(fileExists, bool(const std::string& path));
  MOCK_METHOD1(fileReadToEnd, std::string(const std::string& path));
  MOCK_METHOD1(createThread, Thread::ThreadPtr(std::function<void()> thread_routine));

  Thread::ThreadIdPtr currentThreadId() override {
    return Thread::threadFactoryForTest().currentThreadId();
  }

  std::shared_ptr<Filesystem::MockFile> file_{new Filesystem::MockFile()};
};

class MockOsSysCalls : public OsSysCallsImpl {
public:
  MockOsSysCalls();
  ~MockOsSysCalls();

  // Api::OsSysCalls
  SysCallSizeResult writeFile(int fd, const void* buffer, size_t num_bytes) override;
  SysCallIntResult open(const std::string& full_path, int flags, int mode) override;
  SysCallIntResult setsockopt(SOCKET_FD sockfd, int level, int optname, const void* optval,
                              socklen_t optlen) override;
  SysCallIntResult getsockopt(SOCKET_FD sockfd, int level, int optname, void* optval,
                              socklen_t* optlen) override;

  MOCK_METHOD3(bind, SysCallIntResult(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen));
  MOCK_METHOD3(connect,
               SysCallIntResult(SOCKET_FD sockfd, const sockaddr* addr, socklen_t addrlen));
  MOCK_METHOD3(ioctl, SysCallIntResult(SOCKET_FD sockfd, unsigned long int request, void* argp));
  MOCK_METHOD3(open_, int(const std::string& full_path, int flags, int mode));
  MOCK_METHOD3(writeFile_, ssize_t(int, const void*, size_t));
  MOCK_METHOD3(writeSocket, SysCallSizeResult(SOCKET_FD, const void*, size_t));
  MOCK_METHOD3(writev, SysCallSizeResult(SOCKET_FD, IOVEC*, int));
  MOCK_METHOD3(readv, SysCallSizeResult(SOCKET_FD, IOVEC*, int));
  MOCK_METHOD4(recv, SysCallSizeResult(SOCKET_FD socket, void* buffer, size_t length, int flags));
  MOCK_METHOD1(closeFile, SysCallIntResult(int));
  MOCK_METHOD1(closeSocket, SysCallIntResult(SOCKET_FD));
  MOCK_METHOD3(shmOpen, SysCallIntResult(const char*, int, mode_t));
  MOCK_METHOD1(shmUnlink, SysCallIntResult(const char*));
  MOCK_METHOD2(ftruncate, SysCallIntResult(int fd, off_t length));
  MOCK_METHOD6(mmap, SysCallPtrResult(void* addr, size_t length, int prot, int flags, int fd,
                                      off_t offset));
  MOCK_METHOD2(stat, SysCallIntResult(const char* name, struct stat* stat));
  MOCK_METHOD5(setsockopt_,
               int(SOCKET_FD sockfd, int level, int optname, const void* optval, socklen_t optlen));
  MOCK_METHOD5(getsockopt_,
               int(SOCKET_FD sockfd, int level, int optname, void* optval, socklen_t* optlen));
  MOCK_METHOD3(socket, SysCallSocketResult(int domain, int type, int protocol));
  MOCK_METHOD3(getsockname, SysCallIntResult(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen));
  MOCK_METHOD3(getpeername, SysCallIntResult(SOCKET_FD sockfd, sockaddr* name, socklen_t* namelen));
  MOCK_METHOD1(setSocketNonBlocking, SysCallIntResult(SOCKET_FD sockfd));
  MOCK_METHOD1(setSocketBlocking, SysCallIntResult(SOCKET_FD sockfd));
  MOCK_METHOD2(shutdown, SysCallIntResult(SOCKET_FD sockfd, int how));
  MOCK_METHOD2(listen, SysCallIntResult(SOCKET_FD sockfd, int backlog));
  MOCK_METHOD4(socketpair, SysCallIntResult(int domain, int type, int protocol, SOCKET_FD sv[2]));
  MOCK_METHOD3(accept, SysCallSocketResult(SOCKET_FD sockfd, sockaddr* addr, socklen_t* addr_len));

  size_t num_writes_;
  size_t num_open_;
  Thread::MutexBasicLockable write_mutex_;
  Thread::MutexBasicLockable open_mutex_;
  Thread::CondVar write_event_;
  Thread::CondVar open_event_;
  // Map from (sockfd,level,optname) to boolean socket option.
  using SockOptKey = std::tuple<SOCKET_FD, int, int>;
  std::map<SockOptKey, bool> boolsockopts_;
};

} // namespace Api
} // namespace Envoy
