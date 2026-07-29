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
      lz4Overlay = final: prev: {
        lz4 = prev.lz4.overrideAttrs (old: {
          postInstall = (old.postInstall or "") + ''
            ln -s $out/bin/lz4 $out/bin/lz4c
          '';
        });
      };
      pkgs = import nixpkgs { inherit system; overlays = [ lz4Overlay ]; };
      ccSalt = pkgs.stdenv.cc.suffixSalt;

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

        # FHS sandbox for running Xilinx tools (Vivado, Vitis HLS, etc.)
        # Xilinx installers and binaries expect a conventional /usr/lib layout
        # that NixOS does not provide natively; buildFHSEnv creates that layout.
        # Usage: nix develop .#xilinx
        default = (pkgs.buildFHSEnv {
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
            # Use the *wrapped* toolchain (gcc_multi and binutils are
            # cc-wrapper / binutils-wrapper scripts). The wrappers inject the
            # -B<glibc>/lib, crt paths (Scrt1.o, crti.o), -lgcc_s and
            # dynamic-linker flags by reading the NIX_* env vars set in
            # `profile`. The raw, unwrapped compiler (stdenv.cc.cc) ignores
            # those vars and cannot locate the Nix-store glibc startup files,
            # which breaks host-tool builds like u-boot's fixdep. stdenv.cc.cc
            # is deliberately NOT listed so it doesn't shadow the wrapper at
            # /usr/bin/gcc; stdenv.cc.cc.lib (runtime libs only) is kept.
            gcc_multi
            binutils
            glibc
            glibc.dev
            stdenv.cc.cc.lib
            zstd
            unzip
            nettools
            verilator
            pythonEnv
            gitRepo
            chrpath
            diffstat
            lz4
            rpcsvc-proto
            parted
            git
            clang-tools
            # openssl: used by the Makefile 'root-password' stage to hash the
            # LINUX_EDF_PASSWORD. Having it on PATH here avoids an ad-hoc
            # `nix-shell -p openssl`, whose first-run progress bar can corrupt
            # the terminal.
            openssl
          ];
          profile = ''
            export LD_LIBRARY_PATH=/usr/lib:/usr/lib64:$LD_LIBRARY_PATH

            # BitBake strips unknown env vars before forking subprocesses.
            # The NixOS gcc wrapper needs these to locate glibc startup files
            # (Scrt1.o, crti.o) and libgcc_s in the Nix store — without them
            # host-tool compilation (e.g. u-boot fixdep) fails.
            export NIX_DONT_SET_RPATH_${ccSalt}=1
            export NIX_DYNAMIC_LINKER_${ccSalt}=/lib/ld-linux-x86-64.so.2
            export BB_ENV_PASSTHROUGH_ADDITIONS="NIX_LDFLAGS NIX_CFLAGS_COMPILE NIX_CFLAGS_LINK NIX_CC_WRAPPER_TARGET_HOST_${ccSalt} NIX_DONT_SET_RPATH_${ccSalt} NIX_DYNAMIC_LINKER_${ccSalt}"
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
