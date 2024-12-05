default:
    @just --list

clangd:
    bazel run @hedron_compile_commands//:refresh_all
