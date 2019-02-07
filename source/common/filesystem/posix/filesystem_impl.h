#pragma once

#include <cstdint>
#include <string>

#include "envoy/filesystem/filesystem.h"

namespace Envoy {
namespace Filesystem {

class FileImplPosix : public File {
public:
  FileImplPosix(const std::string& path);
  ~FileImplPosix();

  // Filesystem::File
  Api::SysCallBoolResult open() override;
  Api::SysCallSizeResult write(absl::string_view buffer) override;
  Api::SysCallBoolResult close() override;
  bool isOpen() override;
  std::string path() override;
  std::string errorToString(int error) override;

private:
  int fd_;
  const std::string path_;
  friend class FileSystemImplTest;
};

class InstanceImplPosix : public Instance {
public:
  // Filesystem::Instance
  FilePtr createFile(const std::string& path) override;
  bool fileExists(const std::string& path) override;
  bool directoryExists(const std::string& path) override;
  ssize_t fileSize(const std::string& path) override;
  std::string fileReadToEnd(const std::string& path) override;
  bool illegalPath(const std::string& path) override;

private:
  Api::SysCallStringResult canonicalPath(const std::string& path);
  friend class FileSystemImplTest;
};

} // namespace Filesystem
} // namespace Envoy
