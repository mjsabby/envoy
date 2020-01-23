#include "common/api/os_sys_calls_impl.h"
#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/common/thread_impl.h"
#include "common/filesystem/watcher_impl.h"

namespace Envoy {
namespace Filesystem {

WatcherImpl::WatcherImpl(Event::Dispatcher& dispatcher, Api::Api& api)
    : api_(api), os_sys_calls_(Api::OsSysCallsSingleton::get()) {
  SOCKET_FD socks[2];
  Api::SysCallIntResult result = os_sys_calls_.socketpair(AF_INET, SOCK_STREAM, IPPROTO_TCP, socks);
  ASSERT(result.rc_ == 0);

  event_read_ = socks[0];
  event_write_ = socks[1];
  result = os_sys_calls_.setsocketblocking(event_read_, false);
  ASSERT(result.rc_ == 0);
  result = os_sys_calls_.setsocketblocking(event_write_, false);
  ASSERT(result.rc_ == 0);

  directory_event_ = dispatcher.createFileEvent(
      event_read_,
      [this](uint32_t events) -> void {
        ASSERT(events == Event::FileReadyType::Read);
        onDirectoryEvent();
      },
      Event::FileTriggerType::Level, Event::FileReadyType::Read);

  thread_exit_event_ = ::CreateEvent(nullptr, false, false, nullptr);
  ASSERT(thread_exit_event_ != NULL);
  keep_watching_ = true;
  watch_thread_ = thread_factory_.createThread([this]() -> void { watchLoop(); });
}

WatcherImpl::~WatcherImpl() {
  const BOOL rc = ::SetEvent(thread_exit_event_);
  ASSERT(rc);

  watch_thread_->join();

  for (auto& entry : callback_map_) {
    ::CloseHandle(entry.second->dir_handle_);
    ::CloseHandle(entry.second->overlapped_.hEvent);
  }
  ::CloseHandle(thread_exit_event_);
  ::closesocket(event_read_);
  ::closesocket(event_write_);
}

void WatcherImpl::addWatch(absl::string_view path, uint32_t events, OnChangedCb cb) {
  PathSplitResult result = api_.fileSystem().splitPathFromFilename(path);
  auto dir_narrow = result.directory_;
  auto file_narrow = result.file_;
  // ReadDirectoryChangesW only has a Unicode version, so we need
  // to use wide strings here
  const std::wstring directory = wstring_converter_.from_bytes(std::string(dir_narrow));
  const std::wstring file = wstring_converter_.from_bytes(std::string(file_narrow));

  const HANDLE dir_handle = CreateFileW(
      directory.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
  if (dir_handle == INVALID_HANDLE_VALUE) {
    throw EnvoyException(
        fmt::format("unable to open directory {}: {}", dir_narrow, GetLastError()));
  }
  std::string fii_key(sizeof(FILE_ID_INFO), '\0');
  RELEASE_ASSERT(
      GetFileInformationByHandleEx(dir_handle, FileIdInfo, &fii_key[0], sizeof(FILE_ID_INFO)),
      fmt::format("unable to identify directory {}: {}", dir_narrow, GetLastError()));
  if (callback_map_.find(fii_key) != callback_map_.end()) {
    CloseHandle(dir_handle);
  } else {
    callback_map_[fii_key] = std::make_unique<DirectoryWatch>();
    callback_map_[fii_key]->dir_handle_ = dir_handle;
    callback_map_[fii_key]->buffer_.resize(16384);
    callback_map_[fii_key]->watcher_ = this;

    // According to Microsoft docs, "the hEvent member of the OVERLAPPED structure is not used by
    // the system, so you can use it yourself". We will use it for synchronization of the completion
    // routines
    HANDLE event_handle = ::CreateEvent(nullptr, false, false, nullptr);
    RELEASE_ASSERT(event_handle, fmt::format("CreateEvent failed: {}", GetLastError()));

    callback_map_[fii_key]->overlapped_.hEvent = event_handle;
    dir_watch_complete_events_.push_back(event_handle);

    // send the first ReadDirectoryChangesW request to our watch thread. This ensures that all of
    // the io completion routines will run in that thread
    DWORD rc = ::QueueUserAPC(&issueFirstRead,
                              static_cast<Thread::ThreadImplWin32*>(watch_thread_.get())->handle(),
                              reinterpret_cast<ULONG_PTR>(callback_map_[fii_key].get()));
    RELEASE_ASSERT(rc, fmt::format("QueueUserAPC failed: {}", GetLastError()));

    // wait for issueFirstRead to confirm that it has issued a call to ReadDirectoryChangesW
    rc = ::WaitForSingleObject(event_handle, INFINITE);
    RELEASE_ASSERT(rc == WAIT_OBJECT_0,
                   fmt::format("WaitForSingleObject failed: {}", GetLastError()));

    ENVOY_LOG(debug, "created watch for directory: '{}' handle: {}", dir_narrow, dir_handle);
  }

  callback_map_[fii_key]->watches_.push_back({file, events, cb});
  ENVOY_LOG(debug, "added watch for file '{}' in directory '{}'", file_narrow, dir_narrow);
}

void WatcherImpl::onDirectoryEvent() {
  while (true) {
    char data = 0;
    const int rc = ::recv(event_read_, &data, sizeof(data), 0);
    const int err = ::WSAGetLastError();
    if (rc == SOCKET_ERROR && err == WSAEWOULDBLOCK) {
      return;
    }
    RELEASE_ASSERT(rc != SOCKET_ERROR, fmt::format("recv errored: {}", err));

    if (data == 0) {
      // no callbacks to run; this is just a notification that a DirectoryWatch exited
      return;
    }

    CbClosure callback;
    bool exists = active_callbacks_.try_pop(callback);
    RELEASE_ASSERT(exists, "expected callback, found none");
    ENVOY_LOG(debug, "executing callback");
    callback();
  }
}

void WatcherImpl::issueFirstRead(ULONG_PTR param) {
  DirectoryWatch* dir_watch = reinterpret_cast<DirectoryWatch*>(param);
  // Since the first member in each DirectoryWatch is an OVERLAPPED, we can pass
  // a pointer to DirectoryWatch as the OVERLAPPED for ReadDirectoryChangesW. Then, the
  // completion routine can use its OVERLAPPED* parameter to access the DirectoryWatch see:
  // https://docs.microsoft.com/en-us/windows/desktop/ipc/named-pipe-server-using-completion-routines
  ReadDirectoryChangesW(dir_watch->dir_handle_, &(dir_watch->buffer_[0]),
                        dir_watch->buffer_.capacity(), false,
                        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr,
                        reinterpret_cast<LPOVERLAPPED>(param), &directoryChangeCompletion);

  const BOOL rc = ::SetEvent(dir_watch->overlapped_.hEvent);
  ASSERT(rc);
}

void WatcherImpl::endDirectoryWatch(SOCKET_FD sock, HANDLE event_handle) {
  const BOOL rc = ::SetEvent(event_handle);
  ASSERT(rc);
  // let libevent know that a ReadDirectoryChangesW call returned
  const char data = 0;
  const int bytes_written = ::send(sock, &data, sizeof(data), 0);
  RELEASE_ASSERT(bytes_written == sizeof(data),
                 fmt::format("failed to write 1 byte: {}", ::WSAGetLastError()));
}

void WatcherImpl::directoryChangeCompletion(DWORD err, DWORD num_bytes, LPOVERLAPPED overlapped) {
  DirectoryWatch* dir_watch = reinterpret_cast<DirectoryWatch*>(overlapped);
  WatcherImpl* watcher = dir_watch->watcher_;
  PFILE_NOTIFY_INFORMATION fni = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&dir_watch->buffer_[0]);

  if (err == ERROR_OPERATION_ABORTED) {
    ENVOY_LOG(debug, "ReadDirectoryChangesW aborted, exiting");
    endDirectoryWatch(watcher->event_write_, dir_watch->overlapped_.hEvent);
    return;
  } else if (err != 0) {
    ENVOY_LOG(error, "ReadDirectoryChangesW errored: {}, exiting", err);
    endDirectoryWatch(watcher->event_write_, dir_watch->overlapped_.hEvent);
    return;
  } else if (num_bytes < sizeof(_FILE_NOTIFY_INFORMATION)) {
    ENVOY_LOG(error, "ReadDirectoryChangesW returned {} bytes, expected {}, exiting", num_bytes,
              sizeof(_FILE_NOTIFY_INFORMATION));
    endDirectoryWatch(watcher->event_write_, dir_watch->overlapped_.hEvent);
    return;
  }

  DWORD next_entry = 0;
  do {
    fni = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(reinterpret_cast<char*>(fni) + next_entry);
    // the length of the file name is given in bytes, not wide characters
    std::wstring file(fni->FileName, fni->FileNameLength / 2);
    ENVOY_LOG(debug, "notification: handle: {} action: {:x} file: {}", dir_watch->dir_handle_,
              fni->Action, watcher->wstring_converter_.to_bytes(file));

    uint32_t events = 0;
    if (fni->Action == FILE_ACTION_RENAMED_NEW_NAME) {
      events |= Events::MovedTo;
    }
    if (fni->Action == FILE_ACTION_MODIFIED) {
      events |= Events::Modified;
    }

    for (FileWatch& watch : dir_watch->watches_) {
      if (watch.file_ == file && (watch.events_ & events)) {
        ENVOY_LOG(debug, "matched callback: file: {}", watcher->wstring_converter_.to_bytes(file));
        const auto cb = watch.cb_;
        const auto cb_closure = [cb, events]() -> void { cb(events); };
        watcher->active_callbacks_.push(cb_closure);
        // write a byte to the other end of the socket that libevent is watching
        // this tells the libevent callback to pull this callback off the active_callbacks_
        // queue. We do this so that the callbacks are executed in the main libevent loop,
        // not in this completion routine
        const char data = 1;
        const int bytes_written = ::send(watcher->event_write_, &data, sizeof(data), 0);
        RELEASE_ASSERT(bytes_written == sizeof(data),
                       fmt::format("failed to write 1 byte: {}", ::WSAGetLastError()));
      }
    }

    next_entry = fni->NextEntryOffset;
  } while (next_entry != 0);

  if (!watcher->keep_watching_.load()) {
    ENVOY_LOG(debug, "ending watch on directory: handle: {}", dir_watch->dir_handle_);
    endDirectoryWatch(watcher->event_write_, dir_watch->overlapped_.hEvent);
    return;
  }

  ReadDirectoryChangesW(dir_watch->dir_handle_, &(dir_watch->buffer_[0]),
                        dir_watch->buffer_.capacity(), false,
                        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr,
                        overlapped, directoryChangeCompletion);
}

void WatcherImpl::watchLoop() {
  while (keep_watching_.load()) {
    DWORD wait = WaitForSingleObjectEx(thread_exit_event_, INFINITE, true);
    switch (wait) {
    case WAIT_OBJECT_0:
      // object is getting destroyed, exit the loop
      keep_watching_.store(false);
      break;
    case WAIT_IO_COMPLETION:
      // an IO completion routine finished, nothing to do
      break;
    default:
      ENVOY_LOG(error, "WaitForSingleObjectEx: {}, GetLastError: {}, exiting", wait,
                GetLastError());
      keep_watching_.store(false);
    }
  }

  for (auto& entry : callback_map_) {
    ::CancelIoEx(entry.second->dir_handle_, nullptr);
  }

  const int num_directories = dir_watch_complete_events_.size();
  if (num_directories > 0) {
    while (true) {
      DWORD wait = ::WaitForMultipleObjectsEx(num_directories, &dir_watch_complete_events_[0], true,
                                              INFINITE, true);

      if (WAIT_OBJECT_0 <= wait && wait < (WAIT_OBJECT_0 + num_directories)) {
        // we have no pending IO remaining
        return;
      } else if (wait == WAIT_IO_COMPLETION) {
        // an io completion routine finished, keep waiting
        continue;
      } else {
        ENVOY_LOG(error, "WaitForMultipleObjectsEx: {}, GetLastError: {}, exiting", wait,
                  GetLastError());
        return;
      }
    }
  }
}

} // namespace Filesystem
} // namespace Envoy
