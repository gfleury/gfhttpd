load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "log",
    srcs = [
        "log.c",
    ],
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-g",
    ],
    linkopts = [
        "-g",
    ],
    linkstatic = True,
    visibility = [
        "//main:__pkg__",
        "//main/golang:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/router:__pkg__",
    ],
    deps = [
        "//lib:libevent",
    ],
    alwayslink = 1,
)
