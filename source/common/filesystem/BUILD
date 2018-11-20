licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_cc_platform_dep",
    "envoy_cc_posix_library",
    "envoy_cc_win32_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "directory_lib",
    hdrs = ["directory.h"],
    deps = envoy_cc_platform_dep("directory_iterator_impl_lib"),
)

envoy_cc_win32_library(
    name = "directory_iterator_impl_lib",
    srcs = ["win32/directory_iterator_impl.cc"],
    hdrs = ["win32/directory_iterator_impl.h"],
    strip_include_prefix = "win32",
    deps = [
        "//include/envoy/filesystem:filesystem_interface",
    ],
)

envoy_cc_posix_library(
    name = "directory_iterator_impl_lib",
    srcs = ["posix/directory_iterator_impl.cc"],
    hdrs = ["posix/directory_iterator_impl.h"],
    strip_include_prefix = "posix",
    deps = [
        "//include/envoy/filesystem:filesystem_interface",
        "//source/common/api:os_sys_calls_lib",
    ],
)

envoy_cc_library(
    name = "filesystem_lib",
    srcs = ["filesystem_impl.cc"],
    hdrs = ["filesystem_impl.h"],
    deps = [
        "//include/envoy/api:api_interface",
        "//include/envoy/api:os_sys_calls_interface",
        "//include/envoy/event:dispatcher_interface",
        "//include/envoy/filesystem:filesystem_interface",
        "//source/common/api:os_sys_calls_lib",
        "//source/common/buffer:buffer_lib",
        "//source/common/thread:thread_lib",
    ],
)
