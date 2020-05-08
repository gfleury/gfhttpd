workspace(name = "bazel_study")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

# Rule repository for rules_foreign_cc
http_archive(
    name = "rules_foreign_cc",
    # strip_prefix = "rules_foreign_cc-master",
    strip_prefix = "rules_foreign_cc-FixOSXLinks",
    url = "https://github.com/gfleury/rules_foreign_cc/archive/FixOSXLinks.zip",
    # url = "https://github.com/bazelbuild/rules_foreign_cc/archive/master.zip",
)

load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies([
])

all_content = """filegroup(name = "all", srcs = glob(["**"]), visibility = ["//visibility:public"])"""

# # Repository for OpenSSL
http_archive(
    name = "openssl",
    build_file_content = all_content,
    strip_prefix = "openssl-OpenSSL_1_1_1f",
    urls = [
        "https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.tar.gz",
    ],
)

# Repository for LibEvent
http_archive(
    name = "libevent",
    build_file_content = all_content,
    strip_prefix = "libevent-2.1.11-stable",
    urls = [
        "https://github.com/libevent/libevent/releases/download/release-2.1.11-stable/libevent-2.1.11-stable.tar.gz",
    ],
)

# Repository for nghttp2
http_archive(
    name = "nghttp2",
    build_file_content = all_content,
    sha256 = "82758e13727945f2408d0612762e4655180b039f058d5ff40d055fa1497bd94f",
    strip_prefix = "nghttp2-1.40.0",
    urls = [
        "https://github.com/nghttp2/nghttp2/releases/download/v1.40.0/nghttp2-1.40.0.tar.bz2",
    ],
)

# Repository for pcre2
http_archive(
    name = "pcre2",
    build_file_content = all_content,
    sha256 = "da6aba7ba2509e918e41f4f744a59fa41a2425c59a298a232e7fe85691e00379",
    strip_prefix = "pcre2-10.34",
    urls = [
        "https://ftp.pcre.org/pub/pcre/pcre2-10.34.tar.gz",
    ],
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

# Repository for uthash (used on http3 server impl)
UTHASH_BUILD = """
cc_library(
    name = "uthash_lib",
    hdrs = glob(["src/*.h"]),
    visibility = ["//visibility:public"],
)
"""

new_git_repository(
    name = "uthash",
    build_file_content = UTHASH_BUILD,
    commit = "1124f0a70b0714886402c3c0df03d037e3c4d57a",
    init_submodules = 1,
    remote = "https://github.com/troydhanson/uthash.git",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# BoringSSL
git_repository(
    name = "boringssl",
    # build_file_content = BORINGSSL_BUILD,
    # build_file = "@boringssl",
    commit = "efddb248cb6ca0471ed15764cf60345d417dc497",
    init_submodules = 1,
    remote = "https://github.com/google/boringssl.git",
)

# # Register External Toolchain
# register_toolchains(
#     "//toolchains/x86_64_elf:gcc_linux_x86_64",
# )

# new_local_repository(
#     name = "x86_64_elf_gcc",
#     build_file_content = all_content,
#     path = "/usr/local/Cellar/x86_64-elf-gcc/9.3.0/",
# )

# new_local_repository(
#     name = "x86_64_elf_binutils",
#     build_file_content = all_content,
#     path = "/usr/local/Cellar/x86_64-elf-binutils/2.34/",
# )

# # /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk
# new_local_repository(
#     name = "MacOSXHeaders",
#     build_file_content = all_content,
#     path = "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/",
# )

# Golang
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "142dd33e38b563605f0d20e89d9ef9eda0fc3cb539a14be1bdb1350de2eda659",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.22.2/rules_go-v0.22.2.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.22.2/rules_go-v0.22.2.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

http_archive(
    name = "bazel_gazelle",
    sha256 = "d8c45ee70ec39a57e7a05e5027c32b1576cc7f16d9dd37135b0eddde45cf1b10",
    urls = [
        "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/bazel-gazelle/releases/download/v0.20.0/bazel-gazelle-v0.20.0.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.20.0/bazel-gazelle-v0.20.0.tar.gz",
    ],
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

# Rust
http_archive(
    name = "io_bazel_rules_rust",
    sha256 = "3f6db529492d821a91560c230e2634e34b3e0a3147fc5c4c41ac5bc6ccd45d3f",
    strip_prefix = "rules_rust-fe50d3b54aecbaeac48abdc2ca7cd00a94969e15",
    urls = [
        # Master branch as of 2019-10-07
        "https://github.com/bazelbuild/rules_rust/archive/fe50d3b54aecbaeac48abdc2ca7cd00a94969e15.tar.gz",
    ],
)

load("@io_bazel_rules_rust//rust:repositories.bzl", "rust_repositories")

rust_repositories()

load("@io_bazel_rules_rust//:workspace.bzl", "bazel_version")

bazel_version(name = "bazel_version")

load("//cargo:crates.bzl", "raze_fetch_remote_crates")

raze_fetch_remote_crates()
