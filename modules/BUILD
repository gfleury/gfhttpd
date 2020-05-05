load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "modules",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//main/config:config_headers",
        "//modules/golang",
    ],
    alwayslink = True,
)
