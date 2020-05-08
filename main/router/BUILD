load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "router",
    srcs = [
        "golang.c",
        "router.c",
        "routes.c",
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
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/router/tests:__pkg__",
    ],
    deps = [
        "//main/http_stream",
        "//main/log",
        "//main/config:config_headers",
        "//modules",
        "//lib:pcre2",
        # "//modules/golang:headers",
        "@boringssl//:ssl",
        "@uthash//:uthash_lib",
    ],
)
