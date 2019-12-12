#include "common/common/version.h"

#include <string>

#include "common/common/fmt.h"
#include "common/common/macros.h"
#include "common/common/version_linkstamp.h"

#include "absl/strings/string_view.h"

#ifdef WIN32
// The reason we have this #ifdef on Windows is that build linkstamping doesn't work,
// see: https://github.com/bazelbuild/bazel/issues/6997
// story to fix: #164317493
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

const std::string& VersionInfo::version() {
#ifdef NDEBUG
  static const absl::string_view release_type = "RELEASE";
#else
  static const absl::string_view release_type = "DEBUG";
#endif
#ifdef ENVOY_SSL_VERSION
  static const absl::string_view ssl_version = ENVOY_SSL_VERSION;
#else
  static const absl::string_view ssl_version = "no-ssl";
#endif
  CONSTRUCT_ON_FIRST_USE(std::string,
                         fmt::format("{}/{}/{}/{}/{}", revision(), BUILD_VERSION_NUMBER,
                                     revisionStatus(), release_type, ssl_version));
}
} // namespace Envoy
