bazel test $(bazel query 'kind(".*test", //main/...)') --test_output=errors --verbose_failures --sandbox_debug -c dbg --run_under="valgrind --suppressions=valgrind.suppress --leak-check=full"
cat bazel-testlogs/main/**/tests/tests/test.log  |grep lost
