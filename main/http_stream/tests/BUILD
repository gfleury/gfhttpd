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
        "-Iexternal/uthash/src/",
    ],
    data = [
        "//:valgrind_suppress",
    ],
    linkstatic = True,
    visibility = [
        "//:__pkg__",
    ],
    deps = [
        "//main/http_stream",
        "@uthash//:uthash_lib",
    ],
)
