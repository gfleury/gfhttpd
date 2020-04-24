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
        "-Iexternal/uthash/src/",
    ],
    features = [
        "-pie",
        "fully_static_link",
    ],
    linkopts = [
        "-framework Security",
        "-Wall -Werror -pedantic -fsanitize=address -g",
    ],
    linkstatic = True,
    deps = [
        ":headers",
        "//main/http2",
        "//main/http3",
        "//main/log",
        # Local libs
        "//lib/go/libevent:libevent_lib",
        "//lib/go/libevent:libevent_wrap",
        "//lib/go/dir_index:dir_index_lib",
        # "//lib:dir_index",
        # External deps/libs
        "@uthash//:uthash_lib",
        "@boringssl//:ssl",
        "@boringssl//:crypto",
        "//lib:libevent",
        "//lib:nghttp2",
    ],
)
