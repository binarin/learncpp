{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "nixpkgs";
    flake-parts.url = "flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";
    flake-compat.url = "https://flakehub.com/f/edolstra/flake-compat/1.tar.gz";
  };

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
          env = {
            # CLANGD_FLAGS = "--query-driver=${pkgs.lib.getExe pkgs.gcc14}";
            FLAKE_SELF = "path:${inputs.self}?narHash=${inputs.self.narHash}";
          };
        };
      };
    };
}
