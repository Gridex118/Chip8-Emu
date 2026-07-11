{
  description = "Derivation for CHIP-8 Emulator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
  };

  outputs = { self, nixpkgs, systems, ... }: let
    forEachSystem = f: nixpkgs.lib.genAttrs (import systems) (system: f {
      pkgs = import nixpkgs { inherit system; };
    });
  in {
    packages = forEachSystem ({ pkgs, ... }: {
      default = pkgs.stdenv.mkDerivation {
        pname = "chip8-emu";
        version = "0.1.0";
        src = ./.;
        nativeBuildInputs = with pkgs; [
          cmake
        ];
        buildInputs = with pkgs; [
          sdl2-compat
          nlohmann_json
        ];
        installPhase = ''
          mkdir -p $out/bin
          cp dummy.out $out/bin/chip8-emu
        '';
      };
    });

    apps = forEachSystem ({ pkgs, ... }: {
      default = {
        type = "app";
        program = "${self.packages.${pkgs.system}.default}/bin/chip8-emu";
      };
    });

    devShells = forEachSystem ({ pkgs, ... }: {
      default = pkgs.mkShell {
        packages = with pkgs; [
          sdl2-compat
          nlohmann_json
          pkg-config
          cmake
        ];
      };
    });
  };
}
