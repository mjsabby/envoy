#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>

#undef DELETE
#undef GetMessage

#include "common/common/assert.h"
#include "common/filesystem/filesystem_impl.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "envoy/common/exception.h"

#include "common/common/fmt.h"

namespace Envoy {
namespace Filesystem {

FileImplWin32::FileImplWin32(const std::string& path) : fd_(-1), path_(path) {}

FileImplWin32::~FileImplWin32() {
  const auto result = close();
  ASSERT(result.rc_);
}

Api::SysCallBoolResult FileImplWin32::open() {
  if (isOpen()) {
    return {true, 0};
  }

  const int flags = _O_RDWR | _O_APPEND | _O_CREAT;
  const int mode = _S_IREAD | _S_IWRITE;

  fd_ = ::_open(path_.c_str(), flags, mode);
  if (-1 == fd_) {
    return {false, errno};
  }
  return {true, 0};
}

Api::SysCallSizeResult FileImplWin32::write(absl::string_view buffer) {
  if (fd_ == -1) {
    return {-1, EBADF};
  }

  const ssize_t rc = ::_write(fd_, buffer.data(), buffer.size());
  return {rc, errno};
}

Api::SysCallBoolResult FileImplWin32::close() {
  if (!isOpen()) {
    return {true, 0};
  }

  const int rc = ::_close(fd_);
  if (rc == -1) {
    return {false, errno};
  }

  fd_ = -1;
  return {true, 0};
}

bool FileImplWin32::isOpen() { return fd_ != -1; }

std::string FileImplWin32::path() { return path_; }

std::string FileImplWin32::errorToString(int error) { return ::strerror(error); }

FilePtr InstanceImplWin32::createFile(const std::string& path) {
  return std::make_unique<FileImplWin32>(path);
}

bool InstanceImplWin32::fileExists(const std::string& path) {
  const DWORD attributes = ::GetFileAttributes(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES;
}

bool InstanceImplWin32::directoryExists(const std::string& path) {
  const DWORD attributes = ::GetFileAttributes(path.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return attributes & FILE_ATTRIBUTE_DIRECTORY;
}

ssize_t InstanceImplWin32::fileSize(const std::string& path) {
  struct _stat info;
  if (::_stat(path.c_str(), &info) != 0) {
    return -1;
  }
  return info.st_size;
}

std::string InstanceImplWin32::fileReadToEnd(const std::string& path) {
  if (illegalPath(path)) {
    throw EnvoyException(fmt::format("Invalid path: {}", path));
  }

  std::ios::sync_with_stdio(false);

  // On Windows, we need to explicitly set the file mode as binary. Otherwise,
  // 0x1a will be treated as EOF
  std::ifstream file(path, std::ios_base::binary);
  if (!file) {
    throw EnvoyException(fmt::format("unable to read file: {}", path));
  }

  std::stringstream file_string;
  file_string << file.rdbuf();

  return file_string.str();
}

bool InstanceImplWin32::illegalPath(const std::string& path) {
  // Currently, we don't know of any obviously illegal paths on Windows
  return false;
}

} // namespace Filesystem
} // namespace Envoy
