load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "router",
    srcs = [
        "golang.c",
        "router.c",
    ],
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
        "-Imain/",
        "-Iexternal/uthash/src/",
        "-Ibazel-out/darwin-dbg/bin/main/golang/golang_bridge/",
        "-Ibazel-out/darwin-fastbuild/bin/main/golang/golang_bridge/",
        "-g",
    ],
    linkstatic = True,
    visibility = [
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
    ],
    deps = [
        "//main/golang/golang_bridge:lib_headers",
        "//main/http_stream",
        "//main/log",
        "@boringssl//:ssl",
        "@uthash//:uthash_lib",
    ],
)
