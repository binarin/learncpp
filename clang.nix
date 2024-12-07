let
  pkgs = import <nixpkgs> {};
  llvmPackages = pkgs.llvmPackages_19.override {
    wrapCCWith = args: pkgs.wrapCCWith (args // { gccForLibs = pkgs.gcc14.cc; });
  };
in
 llvmPackages.clang
