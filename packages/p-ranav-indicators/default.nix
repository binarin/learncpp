{stdenvNoCC, fetchFromGitHub, fetchpatch, lib, ...}:
stdenvNoCC.mkDerivation (finalAttrs: {
  pname = "p-ranav-indicators";
  version = "v2.3-dev";
  src = fetchFromGitHub {
    owner = "p-ranav";
    repo = "indicators";
    rev = "9c855c95e7782541a419597242535562fa9e41d7";
    hash = "sha256-lPkxMdvs8kqEobtMahC6BzJvRR1FOVqSdPH3YDeh6Es=";
  };
  phases = "unpackPhase patchPhase installPhase";
  patches = [
    ./small_terminal_width_fix.patch
  ];
  installPhase = ''
    mkdir $out
    cp -r include $out/
    cp -vf "${./display_width.hpp}" $out/include/indicators/display_width.hpp
  '';
})
