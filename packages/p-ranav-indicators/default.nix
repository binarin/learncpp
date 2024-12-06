{stdenvNoCC, fetchFromGitHub, lib, ...}:
stdenvNoCC.mkDerivation (finalAttrs: {
  pname = "p-ranav-indicators";
  version = "v2.3";
  src = fetchFromGitHub {
    owner = "p-ranav";
    repo = "indicators";
    rev = "v2.3";
    hash = "sha256-FA07UbuhsA7HThbyxHxS+V4H5ha0LAXU7sukVfPVpdg=";
  };
  phases = "unpackPhase installPhase";
  installPhase = ''
    mkdir $out
    cp -r include $out/
  '';
})
