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
        packages = {
          p-ranav-indicators = pkgs.callPackage ./packages/p-ranav-indicators {};
        };

        devShells.default = pkgs.mkShell {
          name = "Bazel/C++ devshell";
          packages = with pkgs; [
            clang-tools # goes before clang! otherwise hooks will mess up path
            clang
            bazel_7
            bazel-buildtools
            just
          ];
        };
      };
    };
}
