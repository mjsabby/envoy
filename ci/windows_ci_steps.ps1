#!powershell.exe -Command
bazel.exe --bazelrc=windows/.bazelrc test "@envoy_api//test/build/..."

bazel.exe --bazelrc=windows/.bazelrc build "@boringssl//:ssl"

bazel.exe --bazelrc=windows/.bazelrc build "//external:ares"

bazel.exe --bazelrc=windows/.bazelrc build "//external:event"

bazel.exe --bazelrc=windows/.bazelrc build "//external:yaml_cpp"

# Problem, these don't appear to work on this branch, need to reconfirm;

# bazel.exe --bazelrc=windows/.bazelrc build "//external:nghttp2"

# bazel.exe --bazelrc=windows/.bazelrc build "//external:curl"
