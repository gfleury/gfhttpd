load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "config_headers",
    hdrs = glob(["*.h"]),
    visibility = [
        "//main:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/http_stream:__pkg__",
        "//main/router:__pkg__",
        "//modules:__pkg__",
    ],
)

cc_library(
    name = "config",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-g",
        "-Werror",
        "-Iexternal/uthash/src/",
        "-Iexternal/jsmn/",
        "-Imain/",
    ],
    visibility = [
        "//main:__pkg__",
        "//main/config/tests:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//modules:__pkg__",
    ],
    deps = [
        "//main/log",
        "//main/router",
        "@jsmn//:jsmn_lib",
    ],
)
