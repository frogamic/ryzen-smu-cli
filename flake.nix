{
  description = "CLI utility for altering Ryzen smu params";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { nixpkgs, self, ... }@inputs: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages."${system}";
  in {
    packages."${system}" = {
      default = self.packages."${system}".ryzen-smu-cli;
      ryzen-smu-cli = pkgs.stdenv.mkDerivation (finalAttrs: {
        pname = "ryzen-smu-cli";
        version = "0.0.1";

        src = ./src;

        makeFlags = [
          "LIBSMU_DIR=${pkgs.linuxPackages.ryzen-smu.src}/lib"
          "VERSION=${finalAttrs.version}"
        ];
        installFlags = [ "PREFIX=${placeholder "out"}" ];

        meta = {
          mainProgram = "rsmuctl";
          platforms = [ system ];
        };
      });
    };

    apps."${system}".format = {
      type = "app";
      program = "${pkgs.writeShellScript "format" ''
        ${pkgs.clang-tools}/bin/clang-format --version
        ${pkgs.clang-tools}/bin/clang-format -i --verbose --files=<(ls **/*.[ch])
      ''}";
    };
  };
}
