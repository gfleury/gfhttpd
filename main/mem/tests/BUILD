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
    data = [
        "//:valgrind_suppress",
    ],
    linkstatic = True,
    visibility = [
        "//:__pkg__",
    ],
    deps = [
        "//main/mem",
    ],
)
