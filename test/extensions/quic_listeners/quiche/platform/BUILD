licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_fuzz_test",
    "envoy_cc_test",
    "envoy_cc_test_binary",
    "envoy_cc_test_library",
    "envoy_package",
    "envoy_proto_library",
    "envoy_select_quiche",
)

envoy_package()

envoy_cc_test(
    name = "http2_platform_test",
    srcs = envoy_select_quiche(["http2_platform_test.cc"]),
    external_deps = ["quiche_http2_platform"],
    # not used in CF, so we haven't put the effort in to fix
    test_on_windows = False,
    deps = [
        "//test/test_common:logging_lib",
        "//test/test_common:utility_lib",
    ],
)

envoy_cc_test(
    name = "quic_platform_test",
    srcs = envoy_select_quiche(["quic_platform_test.cc"]),
    data = [
        "//test/extensions/transport_sockets/tls/test_data:certs",
    ],
    external_deps = ["quiche_quic_platform"],
    # not used in CF, so we haven't put the effort in to fix
    test_on_windows = False,
    deps = [
        "//test/extensions/transport_sockets/tls:ssl_test_utils",
        "//test/test_common:logging_lib",
        "//test/test_common:utility_lib",
        "@com_googlesource_quiche//:quic_platform_sleep",
    ],
)

envoy_cc_test(
    name = "spdy_platform_test",
    srcs = envoy_select_quiche(["spdy_platform_test.cc"]),
    external_deps = ["quiche_spdy_platform"],
    # not used in CF, so we haven't put the effort in to fix
    test_on_windows = False,
    deps = [
        "//test/test_common:logging_lib",
        "//test/test_common:utility_lib",
    ],
)
