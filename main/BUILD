load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
)

cc_binary(
    name = "gfhttpd",
    srcs = [
        "gfhttpd.c",
        "ssl.c",
    ],
    copts = [
        "-DHAVE_FCNTL_H",
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    features = [
        "-pie",
        "fully_static_link",
    ],
    linkopts = [
        "-Wall -Werror -pedantic -g",
    ] + select({
        "@bazel_tools//src/conditions:darwin": [
            "-framework Security",
        ],
        "//conditions:default": [
            "-lstdc++",
            "-ldl",
        ],
    }),
    linkstatic = True,
    deps = [
        ":headers",
        "//main/config",
        "//main/http2",
        "//main/http3",
        "//main/log",
        # "//lib:dir_index",
        # External deps/libs
        "@uthash//:uthash_lib",
        "@boringssl//:ssl",
        "@boringssl//:crypto",
        "//lib:libevent",
        "//lib:nghttp2",
        "//modules",
        "//modules/golang",
    ],
)
