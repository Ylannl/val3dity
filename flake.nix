{
  description = "A development shell for val3dity";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          # Build tools
          cmake
          gcc

          # Dependencies
          cgal gmp mpfr
          eigen
          boost
          geos
          nlohmann_json
          spdlog
          pugixml
          tclap
        ];
      };
    };
}
