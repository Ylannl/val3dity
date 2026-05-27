{
  description = "val3dity - validation of 3D GIS primitives";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "aarch64-darwin"
        "aarch64-linux"
        "x86_64-darwin"
        "x86_64-linux"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          lib = pkgs.lib;
        in
        {
          default = pkgs.stdenv.mkDerivation (finalAttrs: {
            pname = "val3dity";
            version = "2.6.0";

            src = lib.cleanSourceWith {
              src = ./.;
              filter =
                path: type:
                let
                  rel = lib.removePrefix "${toString ./.}/" (toString path);
                in
                !(lib.elem rel [
                  "build"
                  "Release"
                  "build-windows"
                ])
                && !(lib.hasPrefix "build/" rel)
                && !(lib.hasPrefix "Release/" rel)
                && !(lib.hasPrefix "build-windows/" rel);
            };

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkg-config
            ];

            buildInputs = [
              pkgs.boost
              pkgs.cgal
              pkgs.eigen
              pkgs.geos
              pkgs.gmp
              pkgs.mpfr
            ];

            cmakeFlags = [
              "-DVAL3DITY_USE_INTERNAL_DEPS=ON"
              "-DCMAKE_BUILD_TYPE=Release"
            ];

            doInstallCheck = true;
            installCheckPhase = ''
              runHook preInstallCheck

              "$out/bin/val3dity" --version
              "$out/bin/val3dity" "$src/data/cityjson/cube.json"

              runHook postInstallCheck
            '';

            meta = {
              description = "Validation of 3D GIS primitives according to ISO19107";
              homepage = "https://github.com/tudelft3d/val3dity";
              license = lib.licenses.gpl3Plus;
              mainProgram = "val3dity";
              platforms = systems;
            };
          });
        }
      );

      apps = forAllSystems (system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/val3dity";
        };
      });

      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.default
            ];

            packages = [
              pkgs.python3Packages.pytest
              pkgs.python3Packages.pyyaml
            ];
          };
        }
      );

      formatter = forAllSystems (system: nixpkgs.legacyPackages.${system}.nixfmt-rfc-style);
    };
}
