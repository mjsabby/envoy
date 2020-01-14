#pragma once

#include <cstdint>
#include <list>
#include <string>

#include "envoy/event/dispatcher.h"
#include "envoy/filesystem/watcher.h"

#include "common/common/linked_object.h"
#include "common/common/logger.h"
#include "common/filesystem/filesystem_impl.h"

namespace Envoy {
namespace Filesystem {

/**
 * Implementation of Watcher that uses kqueue. If the file being watched doesn't exist, we watch
 * the directory, and then try to add a file watch each time there's a write event to the
 * directory.
 */
class WatcherImpl : public Watcher, Logger::Loggable<Logger::Id::file> {
public:
  WatcherImpl(Event::Dispatcher& dispatcher);
  ~WatcherImpl();

  // Filesystem::Watcher
  void addWatch(const std::string& path, uint32_t events, OnChangedCb cb) override;

private:
  struct FileWatch : LinkedObject<FileWatch> {
    ~FileWatch() { close(fd_); }

    int fd_;
    uint32_t events_;
    std::string file_;
    OnChangedCb callback_;
    bool watching_dir_;
  };

  using FileWatchPtr = std::shared_ptr<FileWatch>;

  void onKqueueEvent();
  FileWatchPtr addWatch(const std::string& path, uint32_t events, Watcher::OnChangedCb cb,
                        bool pathMustExist);
  void removeWatch(FileWatchPtr& watch);

  int queue_;
  std::unordered_map<int, FileWatchPtr> watches_;
  Event::FileEventPtr kqueue_event_;
  InstanceImplPosix file_system_;
};

} // namespace Filesystem
} // namespace Envoy
