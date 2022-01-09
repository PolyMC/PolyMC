{
  description = "PolyMC flake";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  inputs.flake-compat = {
    url = "github:edolstra/flake-compat";
    flake = false;
  };
  inputs.flake-utils.url = "github:numtide/flake-utils";
  inputs.libnbtplusplus = {
    url = "github:multimc/libnbtplusplus";
    flake = false;
  };
  inputs.quazip = {
    url = "github:multimc/quazip";
    flake = false;
  };

  outputs = inputs@{ self, nixpkgs, flake-utils, libnbtplusplus, quazip, ... }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
 
        packages = {
          polymc = pkgs.libsForQt5.callPackage ./packages/nix/polymc {
            inherit self;
            submoduleQuazip = quazip;
            submoduleNbt = libnbtplusplus;
          };
        };

        overlay = (final: prev: rec { 
          polymc = prev.libsForQt5.callPackage ./packages/nix/polymc {
            inherit self;
            submoduleQuazip = quazip;
            submoduleNbt = libnbtplusplus;
          };
        });

        apps = {
          polymc = flake-utils.lib.mkApp {
            name = "polymc";
            drv = packages.polymc;
          };
        };
      in
      {
        inherit packages overlay apps;
        defaultPackage = packages.polymc;
        defaultApp = apps.polymc;
      }
    );
}
