licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "os_sys_calls_impl_test",
    srcs = ["os_sys_calls_impl_test.cc"],
    deps = [
        "//source/common/api:os_sys_calls_lib",
        "//test/test_common:environment_lib",
        "//test/test_common:utility_lib",
    ],
)
