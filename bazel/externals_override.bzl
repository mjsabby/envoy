# These options are only used to suppress errors in brought-in QUICHE tests.
# Use #pragma GCC diagnostic ignored in integration code to suppress these errors.
def quiche_copt():
    return select({
        "//bazel:windows_x86_64": [],
        "//conditions:default": [
            # Remove these after upstream fix.
            "-Wno-unused-parameter",
            "-Wno-unused-function",
            "-Wno-type-limits",
            # quic_inlined_frame.h uses offsetof() to optimize memory usage in frames.
            "-Wno-invalid-offsetof",
        ],
    })
