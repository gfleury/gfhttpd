load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
)

cc_library(
    name = "http2",
    srcs = [
        "accept_cb.c",
        "event_cb.c",
        "http2_session.c",
        "read_cb.c",
        "ssl.c",
        "write_cb.c",
    ],
    copts = [
        "-DHAVE_FCNTL_H",
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    linkstatic = True,
    visibility = ["//main:__pkg__"],
    deps = [
        ":headers",
        "//main/router",
        "//main/log",
        "//main/http_stream",
        "@uthash//:uthash_lib",
        "@boringssl//:ssl",
        # "//lib:openssl",
        "//lib:libevent",
        "//lib:nghttp2",
        # Local libs
        "//lib/go/libevent:libevent_lib",
        "//lib/go/libevent:libevent_wrap",
        "//lib/go/dir_index:dir_index_lib",
        # "//lib:dir_index",
    ],
)
