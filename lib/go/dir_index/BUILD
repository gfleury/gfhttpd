load("@io_bazel_rules_go//go:def.bzl", "go_binary")
load("@rules_cc//cc:defs.bzl", "cc_import")

cc_import(
    name = "dir_index_lib",
    static_library = ":dir_index",
    visibility = [
        "//main:__pkg__",
        "//main/http2:__pkg__",
    ],
    # alwayslink = 1,
)

go_binary(
    name = "dir_index",
    srcs = [
        "dir_index.go",
    ],
    cdeps = [
        # "//lib/go/libevent:libevent_lib",
        "//lib:libevent",
    ],
    cgo = True,
    gc_goopts = [
    ],
    importpath = "dir_index",
    linkmode = "c-archive",
    visibility = ["//visibility:public"],
    deps = [
        "//lib/go/libevent",
    ],
)
