load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "router",
    srcs = [
        "module_cb.c",
        "router.c",
    ],
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
        "-Ibazel-out/darwin-dbg/bin/",
        "-Ibazel-out/darwin-fastbuild/",
    ],
    # linkstatic = True,
    visibility = [
        "//main/config:__pkg__",
        "//main/config/tests:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/router/tests:__pkg__",
    ],
    deps = [
        "//main/http_stream",
        "//main/log",
        "//main/config",
        "//modules",
        # "//modules/golang:headers",
        "@boringssl//:ssl",
    ],
)
