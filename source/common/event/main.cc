#include <cstdint>
#include <fstream>

#include "envoy/event/file_event.h"

#include "common/api/os_sys_calls_impl.h"
#include "common/event/dispatcher_impl.h"

#if defined(WIN32)
#include <winsock2.h>
#endif

namespace Envoy {
namespace Event {

class FileEventImplTest {
public:
  FileEventImplTest()
      : os_sys_calls_(Api::OsSysCallsSingleton::get()) {}

  void SetUp() { 
//#if !defined(WIN32)
//    ASSERT(0 == os_sys_calls_.socketpair(AF_UNIX, SOCK_STREAM, 0, fds_).rc_);
//#else
    ASSERT(0 == os_sys_calls_.socketpair(AF_INET, SOCK_STREAM, 0, fds_).rc_);
//#endif
    read_buffer_ = static_cast<char *>(::malloc(1024));
  }

  void TearDown() {
    os_sys_calls_.closeSocket(fds_[0]);
    os_sys_calls_.closeSocket(fds_[1]);
    free(read_buffer_);
  }

  SOCKET_FD test_socket() {
      return fds_[0];
  }

  SOCKET_FD remote_socket() {
      return fds_[1];
  }

  void writeToRemote(const char *buf, size_t len) {
    const Api::SysCallSizeResult result = os_sys_calls_.writeSocket(fds_[1], buf, len);
    ASSERT(len == static_cast<size_t>(result.rc_));
  }
  
  void readFromRemote() {
    const Api::SysCallSizeResult result = os_sys_calls_.recv(fds_[1], read_buffer_, 1024, 0);
    ASSERT(0 != static_cast<size_t>(result.rc_));
    printf("remote server got: %s\n", std::string(read_buffer_, static_cast<size_t>(result.rc_)).c_str());
  }
protected:
  SOCKET_FD fds_[2];
  Api::OsSysCalls& os_sys_calls_;
  char* read_buffer_;
};

} // namespace Event
} // namespace Envoy

int main(int argc, char** argv) {
#if defined(WIN32)
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
  wVersionRequested = MAKEWORD(2, 2);

  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    /* Tell the user that we could not find a usable */
    /* Winsock DLL. */
    printf("WSAStartup failed with error: %d\n", err);
    return 1;
  }
#endif

  using namespace Envoy::Event;
  FileEventImplTest ft;
  ft.SetUp();


  ft.writeToRemote("hi there", 8);

  DispatcherImpl dispatcher;
  {
    StreamEventPtr file_event = dispatcher.createStreamEvent(
      ft.test_socket(),
      [](uv_stream_t *s, ssize_t nread, const uv_buf_t * buf) {
        if (nread >= 0) {
          printf("hi: %s\n", std::string(buf->base, static_cast<size_t>(nread)).c_str());
        } else {
          printf("something bad happened: %Id\n", nread);
        }
      }, 
      [](uv_write_t *w, int status) {
        printf("wrote some data: %d\n", status);
      } 
      );

    char* readBuffer = static_cast<char *>(::malloc(1024));
    uv_buf_t b;
    b.base = readBuffer;
    b.len = 1024;

    file_event->startRead(&b);

    uv_write_t req;
    uv_buf_t wb;
    wb.base = "sending some data";
    wb.len = 17;
    file_event->write(&req, &wb, 1) ;

    dispatcher.run(Dispatcher::RunType::NonBlock);
    ft.readFromRemote();

    std::ofstream file1("/tmp/watching_file1");
    file1 << "hi1";
    std::ofstream file2("/tmp/watching_file2");
    file2 << "hi2";
    WatcherPtr watcher = dispatcher.createFilesystemWatcher();
    watcher->addWatch(
      "/tmp/watching_file1",
      [](const char* filename, int events, int status){
        if (status != 0) {
          printf("watcher error: %d\n", status);
        }
        if (events & Watcher::Events::MovedTo) {
          printf("moved to\n");
        }
        if (events & Watcher::Events::Changed) {
          printf("file changed\n");
        }
      }
    );

    file1 << "bye1";
    dispatcher.run(Dispatcher::RunType::Block);
    file1.close();
    file2.close();

    const BOOL rc = ::MoveFileEx("/tmp/watching_file2", "/tmp/watching_file1", MOVEFILE_REPLACE_EXISTING);
    ASSERT(0 != rc);
    ::Sleep(1000);
    dispatcher.run(Dispatcher::RunType::NonBlock);

  }
  dispatcher.exit();

  ft.TearDown();

  //StreamEventPtr file_event = dispatcher.createFileEvent(
  //ft.,
  //[&](uint32_t events) -> void {
  //if (events & FileReadyType::Read) {
  //    read_event.ready();
  //}

  //if (events & FileReadyType::Write) {
  //    write_event.ready();
  //}
  //},
  //FileTriggerType::Edge, FileReadyType::Read | FileReadyType::Write);

  //dispatcher_.run(Event::Dispatcher::RunType::NonBlock);

}





//class FileEventImplActivateTest : public testing::TestWithParam<Network::Address::IpVersion> {
//public:
//  FileEventImplActivateTest() : os_sys_calls_(Api::OsSysCallsSingleton::get()) {}
//
//protected:
//  Api::OsSysCalls& os_sys_calls_;
//};
//
//INSTANTIATE_TEST_CASE_P(IpVersions, FileEventImplActivateTest,
//                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
//                        TestUtility::ipTestParamsToString);
//
//TEST_P(FileEventImplActivateTest, Activate) {
//  SOCKET_FD fd;
//  if (GetParam() == Network::Address::IpVersion::v4) {
//    fd = os_sys_calls_.socket(AF_INET, SOCK_STREAM, 0).rc_;
//  } else {
//    fd = os_sys_calls_.socket(AF_INET6, SOCK_STREAM, 0).rc_;
//  }
//  ASSERT_TRUE(SOCKET_VALID(fd));
//
//  DangerousDeprecatedTestTime test_time;
//  DispatcherImpl dispatcher(test_time.timeSystem());
//  ReadyWatcher read_event;
//  EXPECT_CALL(read_event, ready()).Times(1);
//  ReadyWatcher write_event;
//  EXPECT_CALL(write_event, ready()).Times(1);
//  ReadyWatcher closed_event;
//  EXPECT_CALL(closed_event, ready()).Times(1);
//
//#if !defined(WIN32)
//  const FileTriggerType trigger = FileTriggerType::Edge;
//#else
//  const FileTriggerType trigger = FileTriggerType::Level;
//#endif
//
//  Event::FileEventPtr file_event = dispatcher.createFileEvent(
//      fd,
//      [&](uint32_t events) -> void {
//        if (events & FileReadyType::Read) {
//          read_event.ready();
//        }
//
//        if (events & FileReadyType::Write) {
//          write_event.ready();
//        }
//
//        if (events & FileReadyType::Closed) {
//          closed_event.ready();
//        }
//      },
//      trigger, FileReadyType::Read | FileReadyType::Write | FileReadyType::Closed);
//
//  file_event->activate(FileReadyType::Read | FileReadyType::Write | FileReadyType::Closed);
//  dispatcher.run(Event::Dispatcher::RunType::NonBlock);
//
//  os_sys_calls_.closeSocket(fd);
//}
//
//TEST_F(FileEventImplTest, EdgeTrigger) {
//#if defined(WIN32)
//  // libevent on Windows doesn't support edge trigger
//  return;
//#endif
//  ReadyWatcher read_event;
//  EXPECT_CALL(read_event, ready()).Times(1);
//  ReadyWatcher write_event;
//  EXPECT_CALL(write_event, ready()).Times(1);
//
//  Event::FileEventPtr file_event = dispatcher_.createFileEvent(
//      fds_[0],
//      [&](uint32_t events) -> void {
//        if (events & FileReadyType::Read) {
//          read_event.ready();
//        }
//
//        if (events & FileReadyType::Write) {
//          write_event.ready();
//        }
//      },
//      FileTriggerType::Edge, FileReadyType::Read | FileReadyType::Write);
//
//  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
//}
//
//TEST_F(FileEventImplTest, LevelTrigger) {
//  ReadyWatcher read_event;
//  EXPECT_CALL(read_event, ready()).Times(2);
//  ReadyWatcher write_event;
//  EXPECT_CALL(write_event, ready()).Times(2);
//
//  int count = 2;
//  Event::FileEventPtr file_event = dispatcher_.createFileEvent(
//      fds_[0],
//      [&](uint32_t events) -> void {
//        if (count-- == 0) {
//          dispatcher_.exit();
//          return;
//        }
//        if (events & FileReadyType::Read) {
//          read_event.ready();
//        }
//
//        if (events & FileReadyType::Write) {
//          write_event.ready();
//        }
//      },
//      FileTriggerType::Level, FileReadyType::Read | FileReadyType::Write);
//
//  dispatcher_.run(Event::Dispatcher::RunType::Block);
//}
//
//TEST_F(FileEventImplTest, SetEnabled) {
//  ReadyWatcher read_event;
//  EXPECT_CALL(read_event, ready()).Times(2);
//  ReadyWatcher write_event;
//  EXPECT_CALL(write_event, ready()).Times(2);
//
//#if !defined(WIN32)
//  const FileTriggerType trigger = FileTriggerType::Edge;
//#else
//  const FileTriggerType trigger = FileTriggerType::Level;
//#endif
//  Event::FileEventPtr file_event =
//      dispatcher_.createFileEvent(fds_[0],
//                                  [&](uint32_t events) -> void {
//                                    if (events & FileReadyType::Read) {
//                                      read_event.ready();
//                                    }
//
//                                    if (events & FileReadyType::Write) {
//                                      write_event.ready();
//                                    }
//#if defined(WIN32)
//                                    dispatcher_.exit();
//                                    return;
//#endif
//                                  },
//                                  trigger, FileReadyType::Read | FileReadyType::Write);
//
//  file_event->setEnabled(FileReadyType::Read);
//  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
//
//  file_event->setEnabled(FileReadyType::Write);
//  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
//
//  file_event->setEnabled(0);
//  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
//
//  file_event->setEnabled(FileReadyType::Read | FileReadyType::Write);
//  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
//}

