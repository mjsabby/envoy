licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_basic_cc_library",
    "envoy_cc_library",
    "envoy_cc_platform_dep",
    "envoy_cc_posix_library",
    "envoy_cc_win32_library",
    "envoy_include_prefix",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "thread_lib",
    hdrs = ["thread.h"],
    external_deps = ["abseil_synchronization"],
    deps = envoy_cc_platform_dep("thread_impl_lib"),
)

envoy_cc_posix_library(
    name = "thread_impl_lib",
    srcs = ["posix/thread_impl.cc"],
    hdrs = ["posix/thread_impl.h"],
    strip_include_prefix = "posix",
    deps = [
        "//include/envoy/thread:thread_interface",
        "//source/common/common:assert_lib",
    ],
)

envoy_cc_win32_library(
    name = "thread_impl_lib",
    srcs = ["win32/thread_impl.cc"],
    hdrs = ["win32/thread_impl.h"],
    strip_include_prefix = "win32",
    deps = [
        "//include/envoy/thread:thread_interface",
        "//source/common/common:assert_lib",
    ],
)
