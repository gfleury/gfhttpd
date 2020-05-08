load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    copts = [
        "-g3",
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    linkopts = [
        "-framework Security",
    ],
    linkstatic = True,
    visibility = [
    ],
    deps = [
        "//main/router",
        "@uthash//:uthash_lib",
    ],
)
