licenses(["notice"])  # Apache 2

config_setting(
    name = "windows_x86_64",
    values = {"cpu": "x64_windows"},
)

cc_library(
    name = "sqlparser",
    srcs = glob(["src/**/*.cpp"]),
    hdrs = glob([
        "include/**/*.h",
        "src/**/*.h",
    ]),
    visibility = ["//visibility:public"],
    defines = select({
        ":windows_x86_64": ["YY_NO_UNISTD_H"],
        "//conditions:default": [],
    }),
)
