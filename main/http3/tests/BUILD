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
        "-Iexternal/raze__quiche__0_4_0/include",
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
        "//lib:libevent",
        "//main/http3",
        "@uthash//:uthash_lib",
    ],
)
