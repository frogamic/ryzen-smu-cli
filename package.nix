{ stdenv, linuxPackages }:

stdenv.mkDerivation (finalAttrs: {
  pname = "ryzen-smu-cli";
  version = "0.0.3";

  src = ./src;

  makeFlags = [
    "LIBSMU_DIR=${linuxPackages.ryzen-smu.src}/lib"
    "VERSION=${finalAttrs.version}"
    "TARGET=${finalAttrs.meta.mainProgram}"
  ];

  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta = {
    mainProgram = "rsmuctl";
    platforms = [ "x86_64-linux" ];
  };
})
