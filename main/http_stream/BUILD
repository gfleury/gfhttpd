load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "http_stream",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-Iexternal/uthash/src/",
    ],
    linkstatic = True,
    visibility = [
        "//lib/golang/golang_bridge:__pkg__",
        "//main/golang:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/http_stream/tests:__pkg__",
        "//main/router:__pkg__",
    ],
    deps = [
        "@uthash//:uthash_lib",
    ],
)
