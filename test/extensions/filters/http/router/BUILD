licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_package",
)
load(
    "//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

envoy_package()

envoy_extension_cc_test(
    name = "config_test",
    srcs = ["config_test.cc"],
    extension_name = "envoy.filters.http.router",
    deps = [
        "//source/extensions/filters/http/router:config",
        "//test/mocks/server:server_mocks",
        "@envoy_api//envoy/config/filter/http/router/v2:pkg_cc_proto",
    ],
)
