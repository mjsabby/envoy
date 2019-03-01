#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>

// <windows.h> uses macros to #define a ton of symbols, two of which (DELETE and GetMessage)
// interfere with our code. DELETE shows up in the base.pb.h header generated from
// api/envoy/api/core/base.proto. Since it's a generated header, we can't #undef DELETE at
// the top of that header to avoid the collision. Similarly, GetMessage shows up in generated
// protobuf code so we can't #undef the symbol there.
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

FileImplWin32::~FileImplWin32() {
  if (isOpen()) {
    const Api::IoCallBoolResult result = close();
    ASSERT(result.rc_);
  }
}

void FileImplWin32::openFile() {
  const int flags = _O_RDWR | _O_APPEND | _O_CREAT;
  const int mode = _S_IREAD | _S_IWRITE;

  fd_ = ::_open(path_.c_str(), flags, mode);
}

ssize_t FileImplWin32::writeFile(absl::string_view buffer) {
  if (fd_ == -1) {
    errno = EBADF;
    return -1;
  }

  return ::_write(fd_, buffer.data(), buffer.size());
}

bool FileImplWin32::closeFile() { return ::_close(fd_) != -1; }

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
  if (file.fail()) {
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
