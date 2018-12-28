#pragma once

#include "envoy/filesystem/filesystem.h"

namespace Envoy {
namespace Filesystem {

class InstanceImplPosix : public Instance {
public:
  bool fileExists(const std::string& path) override;
  bool directoryExists(const std::string& path) override;
  ssize_t fileSize(const std::string& path) override;
  std::string fileReadToEnd(const std::string& path) override;
  bool illegalPath(const std::string& path) override;

  std::string canonicalPath(const std::string& path);
};

} // namespace Filesystem
} // namespace Envoy
