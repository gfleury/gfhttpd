load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "http_stream",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-Iexternal/uthash/src/",
    ],
    # linkstatic = True,
    visibility = [
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/http_stream/tests:__pkg__",
        "//main/router:__pkg__",
        "//modules/golang:__pkg__",
    ],
    deps = [
        "@uthash//:uthash_lib",
    ],
)
