{
  description = "Small C++ library with graphics / resource related utilities used across the hypr* ecosystem";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    hyprutils = {
      url = "github:hyprwm/hyprutils";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.systems.follows = "systems";
    };
  };

  outputs = {
    self,
    nixpkgs,
    systems,
    ...
  } @ inputs: let
    inherit (nixpkgs) lib;
    eachSystem = lib.genAttrs (import systems);
    pkgsFor = eachSystem (system:
      import nixpkgs {
        localSystem.system = system;
        overlays = with self.overlays; [hyprgraphics];
      });
    mkDate = longDate: (lib.concatStringsSep "-" [
      (builtins.substring 0 4 longDate)
      (builtins.substring 4 2 longDate)
      (builtins.substring 6 2 longDate)
    ]);

    version = lib.removeSuffix "\n" (builtins.readFile ./VERSION);
  in {
    overlays = {
      default = self.overlays.hyprgraphics;
      hyprgraphics = lib.composeManyExtensions [
        inputs.hyprutils.overlays.default
        (final: prev: {
          hyprgraphics = final.callPackage ./nix/default.nix {
            stdenv = final.gcc15Stdenv;
            version = version + "+date=" + (mkDate (self.lastModifiedDate or "19700101")) + "_" + (self.shortRev or "dirty");
          };
          hyprgraphics-with-tests = final.hyprgraphics.override {doCheck = true;};
        })
      ];
    };

    packages = eachSystem (system: {
      default = self.packages.${system}.hyprgraphics;
      inherit (pkgsFor.${system}) hyprgraphics hyprgraphics-with-tests;
    });

    formatter = eachSystem (system: pkgsFor.${system}.alejandra);
  };
}
