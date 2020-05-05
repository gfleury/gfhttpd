load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    copts = [
        "-Werror",
        "-Imain/",
    ],
    linkopts = [
        "-framework Security",
    ],
    linkstatic = True,
    visibility = [
    ],
    deps = [
        "//main/router",
    ],
)
