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
    configure_env_vars =
        # Small hack to prevent looking for system packages, but it's not working well on linux due some libevent build hack
        select({
            "@bazel_tools//src/conditions:darwin": {
                "AR": "",
                "OPENSSL_INCS": "-I$EXT_BUILD_ROOT/external/boringssl/src/include/",
                "OPENSSL_LIBADD": "-L$EXT_BUILD_ROOT/external/boringssl/",
                "PKG_CONFIG": "/bin/ls",
            },
            "//conditions:default": {
                "AR": "",
                "OPENSSL_INCS": "-I$EXT_BUILD_ROOT/external/boringssl/src/include/",
                "OPENSSL_LIBADD": "-L$EXT_BUILD_ROOT/bazel-out/k8-fastbuild/bin/external/boringssl",
            },
        }),
    configure_options = [
        "--enable-shared=no",
        "--disable-libevent-regress",
        "--enable-openssl=yes",
        # "--disable-samples",
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
        "//main:__pkg__",
        "//main/http2:__pkg__",
        "//main/http2/tests:__pkg__",
        "//main/http3:__pkg__",
        "//main/http3/tests:__pkg__",
        "//main/log:__pkg__",
        "//modules/golang:__pkg__",
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
        "OPENSSL_CFLAGS": "-I./external/boringssl/src/include",
        "OPENSSL_LIBS": "-lcrypto -lssl",
        "ENABLE_THIRD_PARTY": "true",
        # "LIBEVENT_OPENSSL_CFLAGS": "-Iexternal/libevent",
        # "LIBEVENT_OPENSSL_LIBS": "-levent -levent_openssl",
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

# pcre2 build target
configure_make(
    name = "pcre2",
    configure_env_vars = {
        "AR": "",
        "CXXFLAGS": "-std=c++14",
    },
    configure_options = [
        "--enable-shared=no",
        "--enable-static=yes",
    ],
    lib_source = "@pcre2//:all",
    static_libraries = [
        "libpcre2-8.a",
        "libpcre2-posix.a",
    ],
    visibility = [
        "//main:__pkg__",
        "//main/config:__pkg__",
    ],
    deps = [
    ],
)
