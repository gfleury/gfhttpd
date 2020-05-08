"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""

package(default_visibility = [
    # Public for visibility by "@raze__crate__version//" targets.
    #
    # Prefer access through "//cargo", which limits external
    # visibility to explicit Cargo.toml dependencies.
    "//visibility:public",
])

licenses([
    "restricted",  # "BSD-2-Clause"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

rust_binary(
    name = "quiche_build_script",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        # "boringssl-vendored",
        "default",
    ],
    crate_root = "src/build.rs",
    data = glob(["*"]),
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.0",
    visibility = ["//visibility:private"],
    deps = [
        "@raze__cmake__0_1_42//:cmake",
    ],
)

genrule(
    name = "quiche_build_script_executor",
    srcs = glob([
        "*",
        "**/*.rs",
    ]),
    outs = ["quiche_out_dir_outputs.tar.gz"],
    cmd = "mkdir -p $$(dirname $@)/quiche_out_dir_outputs/;" +
          " (export CARGO_MANIFEST_DIR=\"$$PWD/$$(dirname $(location :Cargo.toml))\";" +
          # TODO(acmcarther): This needs to be revisited as part of the cross compilation story.
          #                   See also: https://github.com/google/cargo-raze/pull/54
          " export TARGET=$$(rustc -vV |grep host|cut -d' ' -f2);" +
          " export RUST_BACKTRACE=1;" +
          #   " export CARGO_FEATURE_BORINGSSL_VENDORED=1;" +
          " export CARGO_FEATURE_DEFAULT=1;" +
          " export OUT_DIR=$$PWD/$$(dirname $@)/quiche_out_dir_outputs;" +
          " export BINARY_PATH=\"$$PWD/$(location :quiche_build_script)\";" +
          " export OUT_TAR=$$PWD/$@;" +
          " export CARGO_FEATURE_STD=1;" +
          " export RUST_BACKTRACE=full;" +
          " export CARGO_FEATURE_ALLOC=1;" +
          " export CARGO_CFG_TARGET_ARCH=x86_64;" +
          " export CARGO_CFG_TARGET_OS=$$(([[ $$(uname) ==  Darwin ]] && echo -n macos) || echo -n linux);" +
          " export CARGO_CFG_TARGET_ENV=gnu;" +
          " export CARGO_MANIFEST_DIR='';" +
          " export OPT_LEVEL=1;" +
          " export HOST=x86_64-$$(([[ $$(uname) ==  Darwin ]] && echo -n macos) || echo -n linux)-gnu;" +
          " export DEBUG=true;" +
          " export PROFILE=debug;" +
          " export GOCACHE=/Users/georgefleury/Library/Caches/go-build;" +
          " export GOPATH=/Users/georgefleury/Desktop/prj/go;" +
          " export GOROOT=/usr/local/Cellar/go/1.14/libexec;" +
          " cd $$(dirname $(location :Cargo.toml)) && $$BINARY_PATH && tar -czf $$OUT_TAR -C $$OUT_DIR .)",
    tags = ["no-sandbox"],
    tools = [
        ":quiche_build_script",
    ],
)

# Unsupported target "client" with type "example" omitted
# Unsupported target "http3-client" with type "example" omitted
# Unsupported target "http3-server" with type "example" omitted
# Unsupported target "qpack-decode" with type "example" omitted
# Unsupported target "qpack-encode" with type "example" omitted
# Unsupported target "quiche" with type "cdylib" omitted

rust_library(
    name = "quiche",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        # "boringssl-vendored",
        "default",
    ],
    crate_root = "src/lib.rs",
    crate_type = "staticlib",
    edition = "2018",
    out_dir_tar = ":quiche_build_script_executor",
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=use_proc_macro",
    ],
    version = "0.3.0",
    deps = [
        "@boringssl//:ssl",
        "@raze__lazy_static__1_4_0//:lazy_static",
        "@raze__libc__0_2_68//:libc",
        "@raze__log__0_4_8//:log",
        "@raze__ring__0_16_12//:ring",
    ],
)

filegroup(
    name = "headers",
    srcs = glob(["include/*.h"]),
)

# Unsupported target "quiche" with type "staticlib" omitted
# Unsupported target "server" with type "example" omitted
