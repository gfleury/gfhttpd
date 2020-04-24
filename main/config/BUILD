load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
)

cc_library(
    name = "http_stream",
    hdrs = glob(["*.h"]),
    visibility = [
        "//main:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
    ],
)
