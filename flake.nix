{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "nixpkgs";
    flake-parts.url = "flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";
  };

  outputs = inputs:
    let
      systems = [ "x86_64-linux" ];
    in inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      inherit systems;
      imports = [
      ];
      perSystem = { pkgs, ... }: {
        devShells.default = pkgs.mkShell {
          name = "nixos-unified-template-shell";
          meta.description = "Shell environment for modifying this Nix configuration";
          packages = with pkgs; [
            clang-tools # goes before clang!
            clang
            bazel_7
            llvmPackages.libstdcxxClang
            bear
          ];
        };
      };
    };
}
