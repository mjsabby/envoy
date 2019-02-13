licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
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
    srcs = select({
        "//bazel:windows_x86_64": [
            "win32/os_sys_calls_impl.cc",
        ],
        "//conditions:default": [
            "linux/os_sys_calls_impl.cc",
        ],
    }),
    hdrs = select({
        "//bazel:windows_x86_64": [
            "win32/os_sys_calls_impl.h",
        ],
        "//conditions:default": [
            "linux/os_sys_calls_impl.h",
        ],
    }),
    strip_include_prefix = select({
        "//bazel:windows_x86_64": "win32",
        "//conditions:default": "linux",
    }),
    deps = [
        "//include/envoy/api:os_sys_calls_interface",
        "//source/common/singleton:threadsafe_singleton",
    ] + select({
        "//bazel:windows_x86_64": ["//source/common/common:assert_lib"],
        "//conditions:default": [],
    }),
)
