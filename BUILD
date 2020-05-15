load(":github_wiki.bzl", "github_wiki")

github_wiki(
    name = "publish_tests_logs",
    testonly = True,
    srcs = glob(["bazel-out/**/testlogs/**/*.log"]),
    clone_url = "https://github.com/gfleury/gfhttpd.wiki.git",
    deps = [
        "//main/config/tests",
        "//main/http_stream/tests",
        "//main/router/tests",
    ],
)

filegroup(
    name = "valgrind_suppress",
    srcs = [
        "valgrind.suppress",
    ],
    visibility = ["//visibility:public"],
)
