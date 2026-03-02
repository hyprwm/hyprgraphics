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

  outputs =
    {
      self,
      nixpkgs,
      systems,
      ...
    }@inputs:
    let
      inherit (nixpkgs) lib;
      eachSystem = lib.genAttrs (import systems);
      pkgsFor = eachSystem (
        system:
        import nixpkgs {
          localSystem.system = system;
          overlays = with self.overlays; [ hyprgraphics-with-deps ];
        }
      );
    in
    {
      overlays = import ./nix/overlays.nix { inherit inputs lib self; };

      packages = eachSystem (system: {
        default = self.packages.${system}.hyprgraphics;
        inherit (pkgsFor.${system}) hyprgraphics hyprgraphics-with-tests;
      });

      formatter = eachSystem (system: pkgsFor.${system}.nixfmt-tree);
    };
}
