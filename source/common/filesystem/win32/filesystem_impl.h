#pragma once

#include <cstdint>
#include <string>

#include "common/filesystem/file_shared_impl.h"
#include "envoy/filesystem/filesystem.h"

namespace Envoy {
namespace Filesystem {

class FileImplWin32 : public FileSharedImpl {
public:
  FileImplWin32(const std::string& path) : FileSharedImpl(path) {}
  ~FileImplWin32();

protected:
  // Filesystem::FileSharedImpl
  void openFile() override;
  ssize_t writeFile(absl::string_view buffer) override;
  bool closeFile() override;

private:
  friend class FileSystemImplTest;
};

class InstanceImplWin32 : public Instance {
public:
  // Filesystem::Instance
  FilePtr createFile(const std::string& path) override;
  bool fileExists(const std::string& path) override;
  bool directoryExists(const std::string& path) override;
  ssize_t fileSize(const std::string& path) override;
  std::string fileReadToEnd(const std::string& path) override;
  bool illegalPath(const std::string& path) override;
};

} // namespace Filesystem
} // namespace Envoy
