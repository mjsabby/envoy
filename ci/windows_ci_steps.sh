#!/usr/bin/bash.exe

set -e

function finish {
  echo "disk space at end of build:"
  df -h
}
trap finish EXIT

echo "disk space at beginning of build:"
df -h

. "$(dirname "$0")"/setup_cache.sh

BAZEL_STARTUP_OPTIONS="--bazelrc=windows/.bazelrc --output_base=c:/_eb"
BAZEL_BUILD_OPTIONS="--config=clang-cl --show_task_finish --verbose_failures \
  --test_output=all ${BAZEL_BUILD_EXTRA_OPTIONS} ${BAZEL_EXTRA_TEST_OPTIONS}"

bazel ${BAZEL_STARTUP_OPTIONS} build ${BAZEL_BUILD_OPTIONS} //bazel/... --build_tag_filters=-skip_on_windows

bazel ${BAZEL_STARTUP_OPTIONS} build ${BAZEL_BUILD_OPTIONS} //source/exe:envoy-static --build_tag_filters=-skip_on_windows

bazel ${BAZEL_STARTUP_OPTIONS} test ${BAZEL_BUILD_OPTIONS} //test/... --test_tag_filters=-skip_on_windows --build_tests_only --test_summary=terse --test_output=errors -- -//test/extensions/... -//test/integration/...
