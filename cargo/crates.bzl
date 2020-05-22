"""
cargo-raze crate workspace functions

DO NOT EDIT! Replaced on runs of cargo-raze
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def _new_http_archive(name, **kwargs):
    if not native.existing_rule(name):
        http_archive(name = name, **kwargs)

def _new_git_repository(name, **kwargs):
    if not native.existing_rule(name):
        new_git_repository(name = name, **kwargs)

def raze_fetch_remote_crates():
    _new_http_archive(
        name = "raze__bumpalo__3_3_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bumpalo/bumpalo-3.3.0.crate",
        type = "tar.gz",
        strip_prefix = "bumpalo-3.3.0",
        build_file = Label("//cargo/remote:bumpalo-3.3.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__cc__1_0_54",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cc/cc-1.0.54.crate",
        type = "tar.gz",
        strip_prefix = "cc-1.0.54",
        build_file = Label("//cargo/remote:cc-1.0.54.BUILD"),
    )

    _new_http_archive(
        name = "raze__cfg_if__0_1_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cfg-if/cfg-if-0.1.10.crate",
        type = "tar.gz",
        strip_prefix = "cfg-if-0.1.10",
        build_file = Label("//cargo/remote:cfg-if-0.1.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__cmake__0_1_44",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cmake/cmake-0.1.44.crate",
        type = "tar.gz",
        strip_prefix = "cmake-0.1.44",
        build_file = Label("//cargo/remote:cmake-0.1.44.BUILD"),
    )

    _new_http_archive(
        name = "raze__js_sys__0_3_39",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/js-sys/js-sys-0.3.39.crate",
        type = "tar.gz",
        strip_prefix = "js-sys-0.3.39",
        build_file = Label("//cargo/remote:js-sys-0.3.39.BUILD"),
    )

    _new_http_archive(
        name = "raze__lazy_static__1_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/lazy_static/lazy_static-1.4.0.crate",
        type = "tar.gz",
        strip_prefix = "lazy_static-1.4.0",
        build_file = Label("//cargo/remote:lazy_static-1.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__libc__0_2_70",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libc/libc-0.2.70.crate",
        type = "tar.gz",
        strip_prefix = "libc-0.2.70",
        build_file = Label("//cargo/remote:libc-0.2.70.BUILD"),
    )

    _new_http_archive(
        name = "raze__libm__0_2_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libm/libm-0.2.1.crate",
        type = "tar.gz",
        strip_prefix = "libm-0.2.1",
        build_file = Label("//cargo/remote:libm-0.2.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__log__0_4_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/log/log-0.4.8.crate",
        type = "tar.gz",
        strip_prefix = "log-0.4.8",
        build_file = Label("//cargo/remote:log-0.4.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__once_cell__1_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/once_cell/once_cell-1.4.0.crate",
        type = "tar.gz",
        strip_prefix = "once_cell-1.4.0",
        build_file = Label("//cargo/remote:once_cell-1.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__proc_macro2__1_0_14",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/proc-macro2/proc-macro2-1.0.14.crate",
        type = "tar.gz",
        strip_prefix = "proc-macro2-1.0.14",
        build_file = Label("//cargo/remote:proc-macro2-1.0.14.BUILD"),
    )

    _new_git_repository(
        name = "raze__quiche__0_4_0",
        remote = "https://github.com/cloudflare/quiche.git",
        commit = "98757caf1f36ac4236174d9ebae09ac027e78df6",
        build_file = Label("//cargo/remote:quiche-0.4.0.BUILD"),
        init_submodules = False,
        patch_args = ["-p1"],
        patches = ["jeeez-cant-fix-this.patch"],
    )

    _new_http_archive(
        name = "raze__quote__1_0_6",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/quote/quote-1.0.6.crate",
        type = "tar.gz",
        strip_prefix = "quote-1.0.6",
        build_file = Label("//cargo/remote:quote-1.0.6.BUILD"),
    )

    _new_http_archive(
        name = "raze__ring__0_16_13",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/ring/ring-0.16.13.crate",
        type = "tar.gz",
        strip_prefix = "ring-0.16.13",
        build_file = Label("//cargo/remote:ring-0.16.13.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde__1_0_110",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde/serde-1.0.110.crate",
        type = "tar.gz",
        strip_prefix = "serde-1.0.110",
        build_file = Label("//cargo/remote:serde-1.0.110.BUILD"),
    )

    _new_http_archive(
        name = "raze__spin__0_5_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/spin/spin-0.5.2.crate",
        type = "tar.gz",
        strip_prefix = "spin-0.5.2",
        build_file = Label("//cargo/remote:spin-0.5.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__syn__1_0_23",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/syn/syn-1.0.23.crate",
        type = "tar.gz",
        strip_prefix = "syn-1.0.23",
        build_file = Label("//cargo/remote:syn-1.0.23.BUILD"),
    )

    _new_http_archive(
        name = "raze__unicode_xid__0_2_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/unicode-xid/unicode-xid-0.2.0.crate",
        type = "tar.gz",
        strip_prefix = "unicode-xid-0.2.0",
        build_file = Label("//cargo/remote:unicode-xid-0.2.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__untrusted__0_7_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/untrusted/untrusted-0.7.1.crate",
        type = "tar.gz",
        strip_prefix = "untrusted-0.7.1",
        build_file = Label("//cargo/remote:untrusted-0.7.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasm_bindgen__0_2_62",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasm-bindgen/wasm-bindgen-0.2.62.crate",
        type = "tar.gz",
        strip_prefix = "wasm-bindgen-0.2.62",
        build_file = Label("//cargo/remote:wasm-bindgen-0.2.62.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasm_bindgen_backend__0_2_62",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasm-bindgen-backend/wasm-bindgen-backend-0.2.62.crate",
        type = "tar.gz",
        strip_prefix = "wasm-bindgen-backend-0.2.62",
        build_file = Label("//cargo/remote:wasm-bindgen-backend-0.2.62.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasm_bindgen_macro__0_2_62",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasm-bindgen-macro/wasm-bindgen-macro-0.2.62.crate",
        type = "tar.gz",
        strip_prefix = "wasm-bindgen-macro-0.2.62",
        build_file = Label("//cargo/remote:wasm-bindgen-macro-0.2.62.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasm_bindgen_macro_support__0_2_62",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasm-bindgen-macro-support/wasm-bindgen-macro-support-0.2.62.crate",
        type = "tar.gz",
        strip_prefix = "wasm-bindgen-macro-support-0.2.62",
        build_file = Label("//cargo/remote:wasm-bindgen-macro-support-0.2.62.BUILD"),
    )

    _new_http_archive(
        name = "raze__wasm_bindgen_shared__0_2_62",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/wasm-bindgen-shared/wasm-bindgen-shared-0.2.62.crate",
        type = "tar.gz",
        strip_prefix = "wasm-bindgen-shared-0.2.62",
        build_file = Label("//cargo/remote:wasm-bindgen-shared-0.2.62.BUILD"),
    )

    _new_http_archive(
        name = "raze__web_sys__0_3_39",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/web-sys/web-sys-0.3.39.crate",
        type = "tar.gz",
        strip_prefix = "web-sys-0.3.39",
        build_file = Label("//cargo/remote:web-sys-0.3.39.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_3_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.3.8.crate",
        type = "tar.gz",
        strip_prefix = "winapi-0.3.8",
        build_file = Label("//cargo/remote:winapi-0.3.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-i686-pc-windows-gnu/winapi-i686-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//cargo/remote:winapi-i686-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-x86_64-pc-windows-gnu/winapi-x86_64-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//cargo/remote:winapi-x86_64-pc-windows-gnu-0.4.0.BUILD"),
    )
