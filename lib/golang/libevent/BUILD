load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "libevent_lib",
    static_library = ":libevent",
    visibility = ["//visibility:public"],
    alwayslink = 1,
)

cc_library(
    name = "libevent_wrap",
    srcs = ["libevent_wrap.c"],
    visibility = ["//visibility:public"],
    deps = [
        "//lib:libevent",
    ],
)

go_library(
    name = "libevent",
    srcs = [
        "libevent.go",
    ],
    cdeps = [
        ":libevent_wrap",
    ],
    cgo = True,
    gc_goopts = [
    ],
    importpath = "libevent",
    # linkmode = "c-archive",
    visibility = ["//visibility:public"],
)
