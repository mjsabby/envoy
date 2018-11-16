#if defined(WIN32)
#include <windows.h>

#endif

#include <cstdint>
#include <fstream>

#include "common/common/assert.h"
#include "common/event/dispatcher_impl.h"
#include "common/filesystem/watcher_impl.h"

#include "test/test_common/environment.h"
#include "test/test_common/test_time.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Filesystem {

class WatcherImplTest : public testing::Test {
protected:
  WatcherImplTest() : dispatcher_(test_time_.timeSystem()) {}

  DangerousDeprecatedTestTime test_time_;
  Event::DispatcherImpl dispatcher_;
};

class WatchCallback {
public:
  MOCK_METHOD1(called, void(uint32_t));
};

TEST_F(WatcherImplTest, All) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_target").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_target").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_link").c_str());

  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test"));
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/watcher_target")); }
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_link"));

  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/watcher_new_target")); }
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_target"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_new_link"));

  WatchCallback callback;
  EXPECT_CALL(callback, called(Watcher::Events::MovedTo)).Times(2);
  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test/watcher_link"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      callback.called(events);
                      dispatcher_.exit();
                    });
  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test/watcher_new_link"),
                          TestEnvironment::temporaryPath("envoy_test/watcher_link"));
  dispatcher_.run(Event::Dispatcher::RunType::Block);

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_target"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_new_link"));
  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test/watcher_new_link"),
                          TestEnvironment::temporaryPath("envoy_test/watcher_link"));
  dispatcher_.run(Event::Dispatcher::RunType::Block);
}

TEST_F(WatcherImplTest, Create) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_target").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/other_file").c_str());

  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test"));
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/watcher_target")); }

  WatchCallback callback;
  EXPECT_CALL(callback, called(Watcher::Events::MovedTo));
  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test/watcher_link"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      callback.called(events);
                      dispatcher_.exit();
                    });

  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/other_file")); }
  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_new_link"));
  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test/watcher_new_link"),
                          TestEnvironment::temporaryPath("envoy_test/watcher_link"));
  dispatcher_.run(Event::Dispatcher::RunType::Block);
}

TEST_F(WatcherImplTest, MultipleFilesInDirectory) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_1").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_2").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_link_1").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_link_2").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_link_1").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test/watcher_new_link_2").c_str());

  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test"));
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/watcher_target_1")); }
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test/watcher_target_2")); }

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_1"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_link_1"));
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_2"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_link_2"));

  WatchCallback callback;
  int times_called = 0;
  EXPECT_CALL(callback, called(Watcher::Events::MovedTo)).Times(2);
  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test/watcher_link_1"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      times_called++;
                      callback.called(events);
                      if (times_called == 2) {
                        dispatcher_.exit();
                      }
                    });

  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test/watcher_link_2"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      times_called++;
                      callback.called(events);
                      if (times_called == 2) {
                        dispatcher_.exit();
                      }
                    });

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_1"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_new_link_1"));
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test/watcher_target_2"),
                             TestEnvironment::temporaryPath("envoy_test/watcher_new_link_2"));

  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test/watcher_new_link_1"),
                          TestEnvironment::temporaryPath("envoy_test/watcher_link_1"));
  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test/watcher_new_link_2"),
                          TestEnvironment::temporaryPath("envoy_test/watcher_link_2"));
  dispatcher_.run(Event::Dispatcher::RunType::Block);
}

TEST_F(WatcherImplTest, MultipleDirectories) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  unlink(TestEnvironment::temporaryPath("envoy_test_1/watcher_target").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test_2/watcher_target").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test_1/watcher_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test_2/watcher_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test_1/watcher_new_link").c_str());
  unlink(TestEnvironment::temporaryPath("envoy_test_2/watcher_new_link").c_str());

  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test_1"));
  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test_2"));
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test_1/watcher_target")); }
  { std::ofstream file(TestEnvironment::temporaryPath("envoy_test_2/watcher_target")); }

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test_1/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test_1/watcher_link"));
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test_2/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test_2/watcher_link"));

  WatchCallback callback;
  int times_called = 0;
  EXPECT_CALL(callback, called(Watcher::Events::MovedTo)).Times(2);
  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test_1/watcher_link"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      times_called++;
                      callback.called(events);
                      if (times_called == 2) {
                        dispatcher_.exit();
                      }
                    });

  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test_1/watcher_link"),
                    Watcher::Events::MovedTo, [&](uint32_t events) -> void {
                      times_called++;
                      callback.called(events);
                      if (times_called == 2) {
                        dispatcher_.exit();
                      }
                    });

  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test_1/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test_1/watcher_new_link"));
  TestUtility::createSymlink(TestEnvironment::temporaryPath("envoy_test_2/watcher_target"),
                             TestEnvironment::temporaryPath("envoy_test_2/watcher_new_link"));

  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test_1/watcher_new_link"),
                          TestEnvironment::temporaryPath("envoy_test_1/watcher_link"));
  TestUtility::renameFile(TestEnvironment::temporaryPath("envoy_test_2/watcher_new_link"),
                          TestEnvironment::temporaryPath("envoy_test_2/watcher_link"));
  dispatcher_.run(Event::Dispatcher::RunType::Block);
}

TEST_F(WatcherImplTest, BadPath) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  EXPECT_THROW(
      watcher->addWatch("this_is_not_a_file", Watcher::Events::MovedTo, [&](uint32_t) -> void {}),
      EnvoyException);

  EXPECT_THROW(watcher->addWatch("this_is_not_a_dir/file", Watcher::Events::MovedTo,
                                 [&](uint32_t) -> void {}),
               EnvoyException);
}

TEST_F(WatcherImplTest, ParentDirectoryRemoved) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

  TestUtility::createDirectory(TestEnvironment::temporaryPath("envoy_test_empty"));

  WatchCallback callback;
  EXPECT_CALL(callback, called(testing::_)).Times(0);

  watcher->addWatch(TestEnvironment::temporaryPath("envoy_test_empty/watcher_link"),
                    Watcher::Events::MovedTo,
                    [&](uint32_t events) -> void { callback.called(events); });

  int rc = rmdir(TestEnvironment::temporaryPath("envoy_test_empty").c_str());
  EXPECT_EQ(0, rc);

  dispatcher_.run(Event::Dispatcher::RunType::NonBlock);
}

TEST_F(WatcherImplTest, RootDirectoryPath) {
  Filesystem::WatcherPtr watcher = dispatcher_.createFilesystemWatcher();

#if !defined(WIN32)
  EXPECT_NO_THROW(watcher->addWatch("/", Watcher::Events::MovedTo, [&](uint32_t) -> void {}));
#else
  EXPECT_NO_THROW(watcher->addWatch("c:\\foo", Watcher::Events::MovedTo, [&](uint32_t) -> void {}));
#endif
}

} // namespace Filesystem
} // namespace Envoy
