load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "router",
    srcs = [
        "router.c",
    ],
    hdrs = glob(["*.h"]),
    linkstatic = True,
    visibility = [
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
    ],
)
