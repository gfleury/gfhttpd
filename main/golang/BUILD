load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "golang",
    srcs = [
        "golang.c",
    ],
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
        "-Ibazel-out/darwin-dbg/bin/lib/golang/golang_bridge/",
    ],
    linkstatic = True,
    visibility = [
        "//main/router:__pkg__",
    ],
    deps = [
        "//lib/golang/golang_bridge:lib_headers",
        "//main/http_stream",
        "//main/log",
        "@boringssl//:ssl",
        "@uthash//:uthash_lib",
    ],
)
