load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = ["test_router.c"] + glob([
        "*.h",
    ]),
    copts = [
        "-g3",
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    data = [
        "//:valgrind_suppress",
    ],
    linkopts = select({
        "@bazel_tools//src/conditions:darwin": [
            "-framework Security",
        ],
        "//conditions:default": [
            "-lstdc++",
            "-ldl",
        ],
    }),
    linkstatic = True,
    visibility = [
        "//:__pkg__",
    ],
    deps = [
        "//main/router",
        "@uthash//:uthash_lib",
    ],
)
