default:
    @just --list

clangd:
    bazel run @hedron_compile_commands//:refresh_all

ds day sample="":
    bazel run "//day$(printf "%02d" "{{ day }}")" < "day$(printf "%02d" "{{ day }}")/sample{{ sample }}.txt"

di day:
    bazel run "//day$(printf "%02d" "{{ day }}")" < "day$(printf "%02d" "{{ day }}")/input.txt"
