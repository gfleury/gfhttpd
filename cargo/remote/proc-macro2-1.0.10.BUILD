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
    name = "proc_macro2_build_script",
    srcs = glob(["**/*.rs"]),
    crate_root = "build.rs",
    edition = "2018",
    deps = [
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    crate_features = [
      "default",
      "proc-macro",
    ],
    data = glob(["*"]),
    version = "1.0.10",
    visibility = ["//visibility:private"],
)

genrule(
    name = "proc_macro2_build_script_executor",
    srcs = glob(["*", "**/*.rs"]),
    outs = ["proc_macro2_out_dir_outputs.tar.gz"],
    tools = [
      ":proc_macro2_build_script",
    ],
    tags = ["no-sandbox"],
    cmd = "mkdir -p $$(dirname $@)/proc_macro2_out_dir_outputs/;"
        + " (export CARGO_MANIFEST_DIR=\"$$PWD/$$(dirname $(location :Cargo.toml))\";"
        # TODO(acmcarther): This needs to be revisited as part of the cross compilation story.
        #                   See also: https://github.com/google/cargo-raze/pull/54
        + " export TARGET='x86_64-apple-darwin';"
        + " export RUST_BACKTRACE=1;"
        + " export CARGO_FEATURE_DEFAULT=1;"
        + " export CARGO_FEATURE_PROC_MACRO=1;"
        + " export OUT_DIR=$$PWD/$$(dirname $@)/proc_macro2_out_dir_outputs;"
        + " export BINARY_PATH=\"$$PWD/$(location :proc_macro2_build_script)\";"
        + " export OUT_TAR=$$PWD/$@;"
        + " cd $$(dirname $(location :Cargo.toml)) && $$BINARY_PATH && tar -czf $$OUT_TAR -C $$OUT_DIR .)"
)

# Unsupported target "features" with type "test" omitted
# Unsupported target "marker" with type "test" omitted

rust_library(
    name = "proc_macro2",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__unicode_xid__0_2_0//:unicode_xid",
    ],
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=use_proc_macro",
    ],
    out_dir_tar = ":proc_macro2_build_script_executor",
    version = "1.0.10",
    crate_features = [
        "default",
        "proc-macro",
    ],
)

# Unsupported target "test" with type "test" omitted
