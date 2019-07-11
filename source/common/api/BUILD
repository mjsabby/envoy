licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_cc_posix_library",
    "envoy_cc_win32_library",
    "envoy_package",
    "envoy_select_hot_restart",
)

envoy_package()

envoy_cc_library(
    name = "api_lib",
    srcs = ["api_impl.cc"],
    hdrs = ["api_impl.h"],
    deps = [
        "//include/envoy/api:api_interface",
        "//source/common/common:thread_lib",
        "//source/common/event:dispatcher_lib",
    ],
)

envoy_cc_library(
    name = "os_sys_calls_lib",
    # This covers bazel:linux_x86_64, linux_aarch64, and linux_ppc
    srcs = [
        "linux/os_sys_calls_impl.cc",
        "linux/os_sys_calls_impl_linux.cc",
    ] + envoy_select_hot_restart(["linux/os_sys_calls_impl_hot_restart.cc"]), 
    hdrs = [
        "linux/os_sys_calls_impl.h",
        "linux/os_sys_calls_impl_linux.h",
    ] + envoy_select_hot_restart(["linux/os_sys_calls_impl_hot_restart.h"]),
    strip_include_prefix = "linux",
    deps = [
        "//include/envoy/api:os_sys_calls_interface",
        "//source/common/singleton:threadsafe_singleton",
    ],
)

envoy_cc_win32_library(
    name = "os_sys_calls_win32_lib",
    srcs = ["win32/os_sys_calls_impl.cc"],
    hdrs = ["win32/os_sys_calls_impl.h"],
    strip_include_prefix = "win32", 
    deps = [
        "//include/envoy/api:os_sys_calls_interface",
        "//source/common/singleton:threadsafe_singleton",
        "//source/common/common:assert_lib",
    ],
)

envoy_cc_posix_library(
    name = "os_sys_calls_posix_lib",
    srcs = ["linux/os_sys_calls_impl.cc"],
    hdrs = ["linux/os_sys_calls_impl.h"],
    strip_include_prefix = "linux", 
    deps = [
        "//include/envoy/api:os_sys_calls_interface",
        "//source/common/singleton:threadsafe_singleton",
    ],
)
