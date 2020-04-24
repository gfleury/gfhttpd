load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_import", "cc_library")

cc_import(
    name = "quiche",
    hdrs = ["@raze__quiche__0_3_0//:headers"],
    static_library = "//cargo:quiche",
    visibility = [
        "//main:__pkg__",
    ],
)

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
)

cc_library(
    name = "http3",
    srcs = [
        "event_cb.c",
        "http3.c",
        "timeout_cb.c",
    ],
    copts = [
        "-DHAVE_FCNTL_H",
        "-Werror",
        "-Iexternal/raze__quiche__0_3_0/include",
        "-Iexternal/uthash/src/",
        "-Imain/",
    ],
    linkstatic = True,
    visibility = ["//main:__pkg__"],
    deps = [
        ":headers",
        ":quiche",
        "//lib:libevent",
        "//main/http_stream",
        "//main/log",
        "//main/router",
        "@uthash//:uthash_lib",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "http3_bin",
    srcs = [
        "main.c",
    ],
    copts = [
        "-DHAVE_FCNTL_H",
        "-Werror",
        "-Iexternal/raze__quiche__0_3_0/include",
        "-Iexternal/uthash/src/",
    ],
    features = [
        "-pie",
        "fully_static_link",
    ],
    linkopts = [
        "-framework Security",
        "-Wall -Werror -pedantic -fsanitize=address -g",
    ],
    linkstatic = True,
    deps = [
        ":headers",
        ":http3",
    ],
)
