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
    "restricted",  # "no license"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_binary",
    "rust_library",
    "rust_test",
)

rust_binary(
    name = "ring_build_script",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "alloc",
        "default",
        "dev_urandom_fallback",
        "lazy_static",
    ],
    crate_root = "build.rs",
    data = glob(["*"]),
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.16.12",
    visibility = ["//visibility:private"],
    deps = [
        "@raze__cc__1_0_50//:cc",
    ],
)

genrule(
    name = "ring_build_script_executor",
    srcs = glob([
        "*",
        "**/*.rs",
    ]),
    outs = ["ring_out_dir_outputs.tar.gz"],
    cmd = "mkdir -p $$(dirname $@)/ring_out_dir_outputs/;" +
          " (export CARGO_MANIFEST_DIR=\"$$PWD/$$(dirname $(location :Cargo.toml))\";" +
          # TODO(acmcarther): This needs to be revisited as part of the cross compilation story.
          #                   See also: https://github.com/google/cargo-raze/pull/54
          " export TARGET=$$(rustc -vV |grep host|cut -d' ' -f2);" +
          " export RUST_BACKTRACE=1;" +
          " export CARGO_FEATURE_ALLOC=1;" +
          " export CARGO_FEATURE_DEFAULT=1;" +
          " export CARGO_FEATURE_DEV_URANDOM_FALLBACK=1;" +
          " export CARGO_FEATURE_LAZY_STATIC=1;" +
          " export OUT_DIR=$$PWD/$$(dirname $@)/ring_out_dir_outputs;" +
          " export BINARY_PATH=\"$$PWD/$(location :ring_build_script)\";" +
          " export OUT_TAR=$$PWD/$@;" +
          " export RUST_BACKTRACE=1;" +
          " export CARGO_PKG_NAME=ring;" +
          " export CARGO_FEATURE_ALLOC=1;" +
          " export CARGO_CFG_TARGET_ARCH=x86_64;" +
          " export CARGO_CFG_TARGET_OS=$$(([[ $$(uname) ==  Darwin ]] && echo -n macos) || echo -n linux);" +
          " export CARGO_CFG_TARGET_ENV=gnu;" +
          " export CARGO_MANIFEST_DIR='';" +
          " export OPT_LEVEL=1;" +
          " export HOST=x86_64-$$(([[ $$(uname) ==  Darwin ]] && echo -n macos) || echo -n linux)-gnu;" +
          " export DEBUG=1;" +
          " cd $$(dirname $(location :Cargo.toml)) && $$BINARY_PATH && tar -czf $$OUT_TAR -C $$OUT_DIR .)",
    tags = ["no-sandbox"],
    tools = [
        ":ring_build_script",
    ],
)

# Unsupported target "aead_tests" with type "test" omitted
# Unsupported target "agreement_tests" with type "test" omitted
# Unsupported target "digest_tests" with type "test" omitted
# Unsupported target "ecdsa_tests" with type "test" omitted
# Unsupported target "ed25519_tests" with type "test" omitted
# Unsupported target "hkdf_tests" with type "test" omitted
# Unsupported target "hmac_tests" with type "test" omitted
# Unsupported target "pbkdf2_tests" with type "test" omitted
# Unsupported target "quic_tests" with type "test" omitted
# Unsupported target "rand_tests" with type "test" omitted

rust_library(
    name = "ring",
    srcs = glob(["**/*.rs"]),
    crate_features = [
        "alloc",
        "default",
        "dev_urandom_fallback",
        "lazy_static",
    ],
    crate_root = "src/lib.rs",
    crate_type = "lib",
    data = glob(["**/*.*"]),
    edition = "2018",
    out_dir_tar = ":ring_build_script_executor",
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=use_proc_macro",
        "-Lnative=./bazel-out/k8-dbg-ST-5e74b77704d3a70b08875590eb0f067cbb9a6e09f41f090f307cf0d79d4b2461/bin/external/raze__ring__0_16_12/ring.out_dir/",
        "-Lnative=./bazel-out/k8-fastbuild-ST-5e74b77704d3a70b08875590eb0f067cbb9a6e09f41f090f307cf0d79d4b2461/bin/external/raze__ring__0_16_12/ring.out_dir/",
        "-Lnative=./bazel-out/darwin-fastbuild-ST-5e74b77704d3a70b08875590eb0f067cbb9a6e09f41f090f307cf0d79d4b2461/bin/external/raze__ring__0_16_12/ring.out_dir/",
        "-Lnative=./bazel-out/darwin-dbg-ST-5e74b77704d3a70b08875590eb0f067cbb9a6e09f41f090f307cf0d79d4b2461/bin/external/raze__ring__0_16_12/ring.out_dir/",
        "-lstatic=ring-core",
        "-lstatic=ring-test",
    ],
    version = "0.16.12",
    deps = [
        "@raze__cc__1_0_50//:cc",
        "@raze__lazy_static__1_4_0//:lazy_static",
        "@raze__libc__0_2_68//:libc",
        "@raze__spin__0_5_2//:spin",
        "@raze__untrusted__0_7_0//:untrusted",
    ],
)

# Unsupported target "rsa_tests" with type "test" omitted
# Unsupported target "signature_tests" with type "test" omitted
