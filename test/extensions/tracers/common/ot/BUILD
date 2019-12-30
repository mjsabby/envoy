licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "opentracing_driver_impl_test",
    srcs = [
        "opentracing_driver_impl_test.cc",
    ],
    # not used in CF, so we haven't put the effort in to fix
    tags = ["skip_on_windows"],
    deps = [
        "//source/extensions/tracers/dynamic_ot:dynamic_opentracing_driver_lib",
        "//test/mocks/http:http_mocks",
        "//test/mocks/stats:stats_mocks",
        "//test/mocks/tracing:tracing_mocks",
        "@io_opentracing_cpp//mocktracer",
    ],
)
