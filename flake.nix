{
  description = "A very basic flake";

  inputs.nixpkgs.url = "nixpkgs";

  inputs.flake-parts.url = "flake-parts";
  inputs.flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";

  inputs.rules_nixpkgs.url = "github:tweag/rules_nixpkgs";
  inputs.rules_nixpkgs.flake = false;

  outputs = inputs:
    let
      systems = [ "x86_64-linux" ];

      overridableNixpkgs = system_def: opts@{config ? {}, overlays ? [], system ? system_def, ...}:
        import inputs.nixpkgs (
          opts // {
            inherit system;
            overlays = [
              (final: prev: {
                p-ranav-indicators = final.callPackage ./packages/p-ranav-indicators {};
                llvmPackages_19_withGcc14 = prev.llvmPackages_19.override {
                  wrapCCWith = args: prev.wrapCCWith (args // { gccForLibs = prev.gcc14.cc; });
                };
                clang_19_withGcc14 = final.llvmPackages_19_withGcc14.clang;
              })
            ] ++ overlays;
          }
        );

    in inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      inherit systems;

      imports = [
      ];

      perSystem = { pkgs, system, config, ... }: {
        _module.args.pkgs = overridableNixpkgs system {};

        legacyPackages.nixpkgsForBazelFn = overridableNixpkgs system;

        packages = {
          inherit (pkgs) p-ranav-indicators;
        };

        devShells.default = pkgs.mkShell {
          name = "Bazel/C++ devshell";
          packages = with pkgs; [
            ((pkgs.llvmPackages_19.override {
              wrapCCWith = args: pkgs.wrapCCWith (args // { gccForLibs = pkgs.gcc14.cc; });
            }).clang-tools)
            bazel_7
            bazel-buildtools
            just
          ];
          shellHook = ''
            cat <<'EOF' > .flake.bazelrc
            common --override_module=rules_nixpkgs_core="${inputs.rules_nixpkgs}/core"
            common --override_module=rules_nixpkgs_cc="${inputs.rules_nixpkgs}/toolchains/cc"
            EOF
          '';
          env = {
            FLAKE_SELF = "path:${inputs.self}?narHash=${inputs.self.narHash}";
          };
        };
      };
    };
}
