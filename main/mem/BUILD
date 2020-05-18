load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mem",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-Werror",
    ],
    # linkstatic = True,
    visibility = [
        "//main/config:__pkg__",
        "//main/http_stream:__pkg__",
        "//main/mem/tests:__pkg__",
    ],
    deps = [
    ],
)
