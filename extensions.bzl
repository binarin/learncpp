load("@rules_nixpkgs_core//:nixpkgs.bzl", "nixpkgs_flake_package")

def p_ranav_indicators():
    nixpkgs_flake_package(
        name = "p-ranav-indicators",
        nix_flake_file = "//:flake.nix",
        nix_flake_file_deps = [
            "//:packages/p-ranav-indicators/default.nix",
            "//:packages/p-ranav-indicators/display_width.hpp",
        ],
        nix_flake_lock_file = "//:flake.lock",
        build_file = "//:packages/p-ranav-indicators/build_file",
        package = "p-ranav-indicators",
    )

def _non_module_dependencies_impl(_ctx):
    p_ranav_indicators()

non_module_dependencies = module_extension(
    implementation = _non_module_dependencies_impl,
)
