#include "common/common/version.h"

#include <string>

#include "common/common/fmt.h"
#include "common/common/macros.h"

#ifdef WIN32
const char build_scm_revision[] = "aabbccdd";
const char build_scm_status[] = "FixMe";
#else
extern const char build_scm_revision[];
extern const char build_scm_status[];
#endif

namespace Envoy {
const std::string& VersionInfo::revision() {
  CONSTRUCT_ON_FIRST_USE(std::string, build_scm_revision);
}
const std::string& VersionInfo::revisionStatus() {
  CONSTRUCT_ON_FIRST_USE(std::string, build_scm_status);
}

std::string VersionInfo::version() {
  return fmt::format("{}/{}/{}/{}/{}", revision(), BUILD_VERSION_NUMBER, revisionStatus(),
#ifdef NDEBUG
                     "RELEASE",
#else
                     "DEBUG",
#endif
                     ENVOY_SSL_VERSION);
}
} // namespace Envoy
