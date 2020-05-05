load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")
load("@rules_cc//cc:defs.bzl", "cc_import")

cc_import(
    name = "golang",
    hdrs = glob(["*.h"]),
    static_library = ":golang_binary",
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_import(
    name = "headers",
    hdrs = glob(["*.h"]),
    # static_library = ":golang_binary",
    visibility = ["//visibility:public"],
    # alwayslink = 1,
)

go_binary(
    name = "golang_binary",
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
    importpath = "golang",
    linkmode = "c-archive",
    visibility = ["//visibility:public"],
    deps = [
        "//modules/golang/go_example",
    ],
)

go_test(
    name = "golang_library_test",
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
    deps = [
        "//modules/golang/go_example",
    ],
    # embed = [":golang_library"],
)
