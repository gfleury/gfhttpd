load("@io_bazel_rules_go//go:def.bzl", "go_binary")
load("@rules_cc//cc:defs.bzl", "cc_import")

cc_import(
    name = "lib_headers",
    hdrs = glob(["*.h"]),
    static_library = ":golang_bridge",
    visibility = ["//visibility:public"],
    alwayslink = 1,
)

go_binary(
    name = "golang_bridge",
    srcs = glob(["*.go"]),
    cdeps = [
        "//lib:libevent",
        "//main/http_stream",
        "@uthash//:uthash_lib",
    ],
    cgo = True,
    copts = [
        "-Imain/",
        "-Iexternal/uthash/src/",
    ],
    importpath = "golang_bridge",
    linkmode = "c-archive",
    visibility = ["//visibility:public"],
)
