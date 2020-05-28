load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "http2",
    srcs = [
        "accept_cb.c",
        "event_cb.c",
        "http2_session.c",
        "listen.c",
        "read_cb.c",
        "ssl.c",
        "write_cb.c",
    ],
    hdrs = glob(["*.h"]),
    copts = [
        "-DHAVE_FCNTL_H",
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    linkstatic = True,
    visibility = [
        "//main:__pkg__",
        "//main/http2/tests:__pkg__",
    ],
    deps = [
        "//main/router",
        "//main/log",
        "//main/http_stream",
        "@uthash//:uthash_lib",
        "@boringssl//:ssl",
        # "//lib:openssl",
        "//lib:libevent",
        "//lib:nghttp2",
    ],
)
