load("@rules_foreign_cc//tools/build_defs:configure.bzl", "configure_make")

# # OpenSSL Build Target
# config_setting(
#     name = "darwin_build",
#     values = {"cpu": "darwin"},
# )

# configure_make(
#     name = "openssl",
#     configure_command = "config",
#     configure_env_vars =
#         select({
#             ":darwin_build": {
#                 "OSX_DEPLOYMENT_TARGET": "10.14",
#                 "AR": "",
#             },
#             "//conditions:default": {},
#         }),
#     configure_options = [
#         "no-shared",
#     ] + select({
#         ":darwin_build": [
#             "enable-ec_nistp_64_gcc_128",
#             "no-ssl2",
#             "no-ssl3",
#             "no-comp",
#         ],
#         "//conditions:default": [],
#     }),
#     lib_source = "@openssl//:all",
#     static_libraries = [
#         "libssl.a",
#         "libcrypto.a",
#     ],
#     visibility = ["//visibility:public"],
# )

# LibEvent build target
configure_make(
    name = "libevent",
    # libevent script uses it's own libtool for linking;
    # so do not specify linker tool for it
    # (otherwise, if the libtool from bazel's toolchain is supplied,
    # the build script has problems with passing output file to libtool)
    # see #315
    configure_env_vars = {
        "AR": "",
    },
    configure_options = [
        "--enable-shared=no",
        "--disable-libevent-regress",
        "--enable-openssl=yes",
    ],
    lib_source = "@libevent//:all",
    out_lib_dir = "lib",
    static_libraries = [
        "libevent_extra.a",
        "libevent.a",
        "libevent_core.a",
        "libevent_openssl.a",
        "libevent_pthreads.a",
    ],
    visibility = [
        "//main/golang/golang_bridge:__pkg__",
        "//main:__pkg__",
        "//main/http2:__pkg__",
        "//main/http3:__pkg__",
        "//main/log:__pkg__",
    ],
    deps = [
        # ":openssl",
        "@boringssl//:crypto",
        "@boringssl//:ssl",
    ],
)

# nghttp2 build target
configure_make(
    name = "nghttp2",
    configure_env_vars = {
        "AR": "",
        "CXXFLAGS": "-std=c++14",
        "OPENSSL_CFLAGS": "-Iexternal/openssl/",
        "OPENSSL_LIBS": "-lcrypto -lssl",
        "LIBEVENT_OPENSSL_CFLAGS": "-Iexternal/libevent",
        "LIBEVENT_OPENSSL_LIBS": "-levent -levent_openssl",
    },
    configure_options = [
        "--enable-shared=no",
        "--enable-static=yes",
    ],
    lib_source = "@nghttp2//:all",
    static_libraries = [
        "libnghttp2.a",
    ],
    visibility = [
        "//main:__pkg__",
        "//main/http2:__pkg__",
    ],
    deps = [
        ":libevent",
        # ":openssl",
        "@boringssl//:ssl",
        "@boringssl//:crypto",
    ],
)
