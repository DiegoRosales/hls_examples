{
  description = "HLS Examples";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    pyproject-nix = {
      url = "github:pyproject-nix/pyproject.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, pyproject-nix, ... }:
    let
      # Xilinx tools only ship for x86_64-linux
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      # Parse requirements.txt and render into a Python environment.
      # validateVersionConstraints is skipped because requirements.txt uses
      # exact pins (==) that are unlikely to match nixpkgs versions exactly.
      project = pyproject-nix.lib.project.loadRequirementsTxt { projectRoot = ./.; };
      # packageOverrides = pkgs.callPackage ./python-packages.nix {};
      # python = pkgs.python312.override { inherit packageOverrides; };
      python = pkgs.python313;
      pythonEnv = python.withPackages (
        project.renderers.withPackages { inherit python; }
      );
    in {
      devShells.${system} = {

        # Default: Python environment for ipyfuse development.
        # All packages are sourced from requirements.txt via pyproject.nix —
        # no venv or pip install step needed.
        # Usage: nix develop
        default = pkgs.mkShell {
          packages = [ pythonEnv pkgs.verilator ];
        };

        # FHS sandbox for running Xilinx tools (Vivado, Vitis HLS, etc.)
        # Xilinx installers and binaries expect a conventional /usr/lib layout
        # that NixOS does not provide natively; buildFHSEnv creates that layout.
        # Usage: nix develop .#xilinx
        xilinx = (pkgs.buildFHSEnv {
          name = "xilinx-env";
          targetPkgs = pkgs: with pkgs; [
            ncurses5
            ncurses
            libxcrypt-legacy
            libpng
            libusb1
            systemd
            pixman
            zlib
            libuuid
            bash
            coreutils
            stdenv.cc.cc
            xorg.libXext
            xorg.libX11
            xorg.libXrender
            xorg.libXtst
            xorg.libXi
            xorg.libXft
            xorg.libxcb
            freetype
            fontconfig
            glib
            gtk2
            gtk3
            graphviz
            gcc
            glibc.dev
            unzip
            nettools
            verilator
            pythonEnv
          ];
          profile = ''
            export LD_LIBRARY_PATH=/usr/lib:/usr/lib64:$LD_LIBRARY_PATH
          '';
          runScript = ''
            env LIBRARY_PATH=/usr/lib \
                CMAKE_LIBRARY_PATH=/usr/lib \
                CMAKE_INCLUDE_PATH=/usr/include \
                bash
          '';
        }).env;

      };
    };
}
