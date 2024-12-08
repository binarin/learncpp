load("@rules_nixpkgs_core//:nixpkgs.bzl", "nixpkgs_local_repository")

def _nixpkgs_from_flake_impl(module_ctx):
    flake_self = module_ctx.os.environ.get("FLAKE_SELF", "!MUST_BE_SET!")
    nixpkgs_local_repository(
        name = "nixpkgs",
        nix_file_content = '''(builtins.getFlake "{}").legacyPackages."x86_64-linux".nixpkgsForBazelFn'''.format(flake_self),
    )

nixpkgs_from_flake = module_extension(
    implementation = _nixpkgs_from_flake_impl,
    environ = [ "FLAKE_SELF" ],
)
