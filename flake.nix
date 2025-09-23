{
  description = "avUtils - Utility library for avBuilder";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    packages.${system}.default = pkgs.stdenv.mkDerivation {
      pname = "avUtils";
      version = "0.1.0";
      src = ./.;

      # Assuming avUtils is just source files or headers, no build required
      buildPhase = ''
        echo "No build required"
      '';
      installPhase = ''
        mkdir -p $out
        cp -r ./* $out/
      '';

      meta = with pkgs.lib; {
        description = "Utility library for avBuilder";
        license = licenses.mit;
        maintainers = [];
      };
    };

    defaultPackage = self.packages.${system}.default;
  };
}

