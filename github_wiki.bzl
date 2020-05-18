#
# Github wiki page publisher
#

def _github_wiki_impl(ctx):
    clonedir = ctx.outputs.clonedir

    output = []

    for f in ctx.files.srcs:
        file = ctx.actions.declare_file(ctx.attr.dest + f.path.replace("/", "-").replace(".log", ".md"))
        ctx.actions.run_shell(
            inputs = [f],
            outputs = [file],
            progress_message = "Generating files of %s" % file.short_path,
            command = "echo '```' > {} && ".format(file.path) +
                      "cat {} >> {} && ".format(f.path, file.path) +
                      "echo '```' >> {} ".format(file.path),
            execution_requirements = {
                "no-sandbox": "1",
                "no-cache": "1",
                "no-remote": "1",
                "local": "1",
            },
        )

        output.append(file)

    if len(output) > 0:
        ctx.actions.run_shell(
            inputs = output,
            outputs = [clonedir],
            progress_message = "Commiting the changes on %s" % clonedir.short_path,
            command = "export HOME=\"$PWD\" && git config --global user.email \"{}\" && git config --global user.name \"{}\" &&".format(ctx.attr.git_email, ctx.attr.git_name) +
                      "git clone {} {} && ".format(ctx.attr.clone_url, clonedir.path) +
                      "cp -Lrf {}/* {} && ".format(output[0].dirname, clonedir.path) +
                      "cd {} && ".format(clonedir.path) +
                      "git add * && git commit -a -m '\''Commit msg'\'' && git push || true",
        )

    return [DefaultInfo(files = depset([clonedir]))]

github_wiki = rule(
    implementation = _github_wiki_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            mandatory = True,
            doc = "The file whose are to be published",
        ),
        "clone_url": attr.string(
            mandatory = True,
            doc = "Git url to clone",
        ),
        "dest": attr.string(
            default = "wiki/",
            doc = "Destination dir to ship",
        ),
        "deps": attr.label_list(
            default = [],
        ),
        "git_name": attr.string(
            default = "Wiki",
        ),
        "git_email": attr.string(
            default = "root@localhost",
        ),
    },
    outputs = {"clonedir": "%{name}-wiki"},
)
