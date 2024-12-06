let
  lock = builtins.fromJSON (builtins.readFile ./flake.lock);
  spec = lock.nodes.nixpkgs.locked;
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/archive/${spec.rev}.tar.gz";
in
opts:
import nixpkgs ({
  overlays = [
    (final: prev: {
      p-ranav-indicators = final.callPackage ./packages/p-ranav-indicators {};
    })
  ];
} // opts)
