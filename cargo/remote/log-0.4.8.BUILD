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
  "restricted", # "MIT OR Apache-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)

rust_binary(
    name = "log_build_script",
    srcs = glob(["**/*.rs"]),
    crate_root = "build.rs",
    edition = "2015",
    deps = [
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    crate_features = [
      "serde",
      "std",
    ],
    data = glob(["*"]),
    version = "0.4.8",
    visibility = ["//visibility:private"],
)

genrule(
    name = "log_build_script_executor",
    srcs = glob(["*", "**/*.rs"]),
    outs = ["log_out_dir_outputs.tar.gz"],
    tools = [
      ":log_build_script",
    ],
    tags = ["no-sandbox"],
    cmd = "mkdir -p $$(dirname $@)/log_out_dir_outputs/;"
        + " (export CARGO_MANIFEST_DIR=\"$$PWD/$$(dirname $(location :Cargo.toml))\";"
        # TODO(acmcarther): This needs to be revisited as part of the cross compilation story.
        #                   See also: https://github.com/google/cargo-raze/pull/54
        + " export TARGET='x86_64-apple-darwin';"
        + " export RUST_BACKTRACE=1;"
        + " export CARGO_FEATURE_SERDE=1;"
        + " export CARGO_FEATURE_STD=1;"
        + " export OUT_DIR=$$PWD/$$(dirname $@)/log_out_dir_outputs;"
        + " export BINARY_PATH=\"$$PWD/$(location :log_build_script)\";"
        + " export OUT_TAR=$$PWD/$@;"
        + " cd $$(dirname $(location :Cargo.toml)) && $$BINARY_PATH && tar -czf $$OUT_TAR -C $$OUT_DIR .)"
)

# Unsupported target "filters" with type "test" omitted

rust_library(
    name = "log",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__cfg_if__0_1_10//:cfg_if",
        "@raze__serde__1_0_110//:serde",
    ],
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=use_proc_macro",
    ],
    out_dir_tar = ":log_build_script_executor",
    version = "0.4.8",
    crate_features = [
        "serde",
        "std",
    ],
)

