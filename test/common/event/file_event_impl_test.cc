#include <cstdint>

#include "envoy/event/file_event.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/event/dispatcher_impl.h"
#include "common/stats/isolated_store_impl.h"

#include "test/mocks/common.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Event {
namespace {

class FileEventImplTest : public testing::Test {
public:
  FileEventImplTest()
      : api_(Api::createApiForTest()), dispatcher_(api_->allocateDispatcher()),
        os_sys_calls_(Api::OsSysCallsSingleton::get()) {}

  void SetUp() override {
#ifdef WIN32
    ASSERT_EQ(0, os_sys_calls_.socketpair(AF_INET, SOCK_STREAM, 0, fds_).rc_);
#else
    ASSERT_EQ(0, os_sys_calls_.socketpair(AF_UNIX, SOCK_DGRAM, 0, fds_).rc_);
#endif
    int data = 1;

    const Api::SysCallSizeResult result = os_sys_calls_.writeSocket(fds_[1], &data, sizeof(data));
    ASSERT_EQ(sizeof(data), static_cast<size_t>(result.rc_));
  }

  void TearDown() override {
#ifdef WIN32
    ::closesocket(fds_[0]);
    ::closesocket(fds_[1]);
#else
    ::close(fds_[0]);
    ::close(fds_[1]);
#endif
  }

protected:
  SOCKET_FD fds_[2];
  Api::ApiPtr api_;
  DispatcherPtr dispatcher_;
  Api::OsSysCalls& os_sys_calls_;
};

class FileEventImplActivateTest : public testing::TestWithParam<Network::Address::IpVersion> {
public:
  FileEventImplActivateTest() : os_sys_calls_(Api::OsSysCallsSingleton::get()) {}

protected:
  Api::OsSysCalls& os_sys_calls_;
};

INSTANTIATE_TEST_SUITE_P(IpVersions, FileEventImplActivateTest,
                         testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                         TestUtility::ipTestParamsToString);

TEST_P(FileEventImplActivateTest, Activate) {
  SOCKET_FD fd;
  if (GetParam() == Network::Address::IpVersion::v4) {
    fd = os_sys_calls_.socket(AF_INET, SOCK_STREAM, 0).rc_;
  } else {
    fd = os_sys_calls_.socket(AF_INET6, SOCK_STREAM, 0).rc_;
  }
  ASSERT_TRUE(SOCKET_VALID(fd));

  Api::ApiPtr api = Api::createApiForTest();
  DispatcherPtr dispatcher(api->allocateDispatcher());
  ReadyWatcher read_event;
  EXPECT_CALL(read_event, ready()).Times(1);
  ReadyWatcher write_event;
  EXPECT_CALL(write_event, ready()).Times(1);
  ReadyWatcher closed_event;
  EXPECT_CALL(closed_event, ready()).Times(1);

#ifdef WIN32
  const FileTriggerType trigger = FileTriggerType::Level;
#else
  const FileTriggerType trigger = FileTriggerType::Edge;
#endif

  Event::FileEventPtr file_event = dispatcher->createFileEvent(
      fd,
      [&](uint32_t events) -> void {
        if (events & FileReadyType::Read) {
          read_event.ready();
        }

        if (events & FileReadyType::Write) {
          write_event.ready();
        }

        if (events & FileReadyType::Closed) {
          closed_event.ready();
        }
      },
      trigger, FileReadyType::Read | FileReadyType::Write | FileReadyType::Closed);

  file_event->activate(FileReadyType::Read | FileReadyType::Write | FileReadyType::Closed);
  dispatcher->run(Event::Dispatcher::RunType::NonBlock);

#ifdef WIN32
  ::closesocket(fd);
#else
  ::close(fd);
#endif
}

TEST_F(FileEventImplTest, EdgeTrigger) {
#ifdef WIN32
  // libevent on Windows doesn't support edge trigger
  return;
#endif
  ReadyWatcher read_event;
  EXPECT_CALL(read_event, ready()).Times(1);
  ReadyWatcher write_event;
  EXPECT_CALL(write_event, ready()).Times(1);

  Event::FileEventPtr file_event = dispatcher_->createFileEvent(
      fds_[0],
      [&](uint32_t events) -> void {
        if (events & FileReadyType::Read) {
          read_event.ready();
        }

        if (events & FileReadyType::Write) {
          write_event.ready();
        }
      },
      FileTriggerType::Edge, FileReadyType::Read | FileReadyType::Write);

  dispatcher_->run(Event::Dispatcher::RunType::NonBlock);
}

TEST_F(FileEventImplTest, LevelTrigger) {
  ReadyWatcher read_event;
  EXPECT_CALL(read_event, ready()).Times(2);
  ReadyWatcher write_event;
  EXPECT_CALL(write_event, ready()).Times(2);

  int count = 2;
  Event::FileEventPtr file_event = dispatcher_->createFileEvent(
      fds_[0],
      [&](uint32_t events) -> void {
        if (count-- == 0) {
          dispatcher_->exit();
          return;
        }
        if (events & FileReadyType::Read) {
          read_event.ready();
        }

        if (events & FileReadyType::Write) {
          write_event.ready();
        }
      },
      FileTriggerType::Level, FileReadyType::Read | FileReadyType::Write);

  dispatcher_->run(Event::Dispatcher::RunType::Block);
}

TEST_F(FileEventImplTest, SetEnabled) {
  ReadyWatcher read_event;
  EXPECT_CALL(read_event, ready()).Times(2);
  ReadyWatcher write_event;
  EXPECT_CALL(write_event, ready()).Times(2);

#ifdef WIN32
  const FileTriggerType trigger = FileTriggerType::Level;
#else
  const FileTriggerType trigger = FileTriggerType::Edge;
#endif
  Event::FileEventPtr file_event =
      dispatcher_->createFileEvent(fds_[0],
                                   [&](uint32_t events) -> void {
                                     if (events & FileReadyType::Read) {
                                       read_event.ready();
                                     }

                                     if (events & FileReadyType::Write) {
                                       write_event.ready();
                                     }
#ifdef WIN32
                                     dispatcher_->exit();
                                     return;
#endif
                                   },
                                   trigger, FileReadyType::Read | FileReadyType::Write);

  file_event->setEnabled(FileReadyType::Read);
  dispatcher_->run(Event::Dispatcher::RunType::NonBlock);

  file_event->setEnabled(FileReadyType::Write);
  dispatcher_->run(Event::Dispatcher::RunType::NonBlock);

  file_event->setEnabled(0);
  dispatcher_->run(Event::Dispatcher::RunType::NonBlock);

  file_event->setEnabled(FileReadyType::Read | FileReadyType::Write);
  dispatcher_->run(Event::Dispatcher::RunType::NonBlock);
}

} // namespace
} // namespace Event
} // namespace Envoy
