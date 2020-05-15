load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    copts = [
        "-g",
        "-Werror",
        "-Iexternal/uthash/src/",
        "-Imain/",
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
        "//main/config",
        "//main/router",
    ],
)
