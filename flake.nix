{
  description = "CLI utility for altering Ryzen smu params";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { nixpkgs, self, ... }@inputs: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages."${system}";
    libsmu = "${pkgs.linuxPackages.ryzen-smu.src}/lib";
  in {
    packages."${system}" = {
      default = self.packages."${system}".ryzen-smu-cli;
      ryzen-smu-cli = pkgs.stdenv.mkDerivation (finalAttrs: {
        pname = "ryzen-smu-cli";
        version = "0.0.2";

        src = ./src;

        makeFlags = [
          "LIBSMU_DIR=${libsmu}"
          "VERSION=${finalAttrs.version}"
          "TARGET=${finalAttrs.meta.mainProgram}"
        ];
        installFlags = [ "PREFIX=${placeholder "out"}" ];

        meta = {
          mainProgram = "rsmuctl";
          platforms = [ system ];
        };
      });
    };

    apps."${system}" = {
      format = {
        type = "app";
        program = "${pkgs.writeShellScript "format" ''
          ${pkgs.clang-tools}/bin/clang-format --version
          ${pkgs.clang-tools}/bin/clang-format -i --verbose src/*.[ch]
        ''}";
      };
      iwyu = {
        type = "app";
        program = let
          inherit (nixpkgs.lib) join;
          iwyu = pkgs.include-what-you-use;
        in "${pkgs.writeShellScript "iwyu" ''
          ${iwyu}/bin/include-what-you-use --version
          rm -f compile_commands.json
          ${pkgs.bear}/bin/bear -- ${pkgs.gnumake}/bin/make -C src clean all \
          CC=${pkgs.gcc}/bin/gcc \
          ${join " " self.packages."${system}".default.makeFlags}
          ${iwyu}/bin/iwyu_tool.py -p .
        ''}";
      };
    };
  };
}
