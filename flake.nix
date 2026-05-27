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
          cleanSrc = lib.cleanSourceWith {
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
                "build-wasm"
              ])
              && !(lib.hasPrefix "build/" rel)
              && !(lib.hasPrefix "Release/" rel)
              && !(lib.hasPrefix "build-windows/" rel)
              && !(lib.hasPrefix "build-wasm/" rel);
          };
          wasmBuild = pkgs.writeShellApplication {
            name = "wasm-build";
            runtimeInputs = [
              pkgs.cmake
              pkgs.emscripten
              pkgs.ninja
              pkgs.nodejs
              pkgs.pkg-config
            ];
            text = ''
              set -euo pipefail

              root="$(pwd)"
              build_dir="''${VAL3DITY_WASM_BUILD_DIR:-$root/build-wasm}"
              deps_dir="$build_dir/deps"
              geos_prefix="$deps_dir/geos"

              export EM_CACHE="''${EM_CACHE:-$build_dir/emscripten-cache}"
              mkdir -p "$build_dir" "$deps_dir" "$EM_CACHE"

              if [ ! -f "$geos_prefix/lib/cmake/GEOS/geos-config.cmake" ] && [ ! -f "$geos_prefix/lib/cmake/geos/geos-config.cmake" ]; then
                emcmake cmake \
                  -S ${pkgs.geos.src} \
                  -B "$deps_dir/geos-build" \
                  -G Ninja \
                  -DCMAKE_BUILD_TYPE=Release \
                  -DCMAKE_INSTALL_PREFIX="$geos_prefix" \
                  -DBUILD_SHARED_LIBS=OFF \
                  -DBUILD_TESTING=OFF \
                  -DBUILD_GEOSOP=OFF \
                  -DGEOS_BUILD_BENCHMARKS=OFF \
                  -DGEOS_BUILD_DEVELOPER=OFF \
                  -DGEOS_BUILD_TESTS=OFF \
                  -DGEOS_ENABLE_TESTS=OFF
                cmake --build "$deps_dir/geos-build"
                cmake --install "$deps_dir/geos-build"
              fi

              emcmake cmake \
                -S "$root" \
                -B "$build_dir" \
                -G Ninja \
                -DVAL3DITY_WASM=ON \
                -DVAL3DITY_LIBRARY=OFF \
                -DVAL3DITY_USE_INTERNAL_DEPS=ON \
                -DCGAL_DISABLE_GMP=ON \
                -DCGAL_DIR=${pkgs.cgal}/lib/cmake/CGAL \
                -DEigen3_DIR=${pkgs.eigen}/share/eigen3/cmake \
                -DGEOS_DIR="$geos_prefix/lib/cmake/GEOS" \
                -DBoost_DIR=${pkgs.boost.dev}/lib/cmake/Boost-${pkgs.boost.version} \
                -Dboost_headers_DIR=${pkgs.boost.dev}/lib/cmake/boost_headers-${pkgs.boost.version} \
                -DBoost_INCLUDE_DIR=${pkgs.boost.dev}/include \
                -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_PREFIX_PATH="$geos_prefix;${pkgs.cgal};${pkgs.eigen};${pkgs.boost.dev}"
              cmake --build "$build_dir" --target val3dity_wasm
              cp "$root/wasm/demo.html" "$build_dir/demo.html"
              cp "$root/wasm/val3dity.js" "$build_dir/val3dity.js"

              echo "Built $build_dir/val3dity_wasm.mjs"
              echo "Built $build_dir/val3dity_wasm.wasm"
            '';
          };
        in
        {
          wasm-build = wasmBuild;

          default = pkgs.stdenv.mkDerivation (finalAttrs: {
            pname = "val3dity";
            version = "2.6.0";

            src = cleanSrc;

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

          wasm = pkgs.stdenv.mkDerivation {
            pname = "val3dity-wasm";
            version = "2.6.0";

            src = cleanSrc;

            nativeBuildInputs = [
              wasmBuild
            ];

            buildPhase = ''
              runHook preBuild

              export HOME="$TMPDIR"
              export EM_CACHE="$TMPDIR/emscripten-cache"
              wasm-build

              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall

              mkdir -p "$out/share/val3dity/wasm"
              cp build-wasm/val3dity_wasm.mjs "$out/share/val3dity/wasm/"
              cp build-wasm/val3dity_wasm.wasm "$out/share/val3dity/wasm/"
              cp build-wasm/demo.html "$out/share/val3dity/wasm/"
              cp build-wasm/val3dity.js "$out/share/val3dity/wasm/"
              cp wasm/README.md "$out/share/val3dity/wasm/"

              runHook postInstall
            '';

            meta = {
              description = "WebAssembly bindings for val3dity";
              homepage = "https://github.com/tudelft3d/val3dity";
              license = lib.licenses.gpl3Plus;
              platforms = systems;
            };
          };
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

          wasm = pkgs.mkShell {
            packages = [
              self.packages.${system}.wasm-build
              pkgs.cmake
              pkgs.emscripten
              pkgs.ninja
              pkgs.nodejs
              pkgs.pkg-config
            ];
          };
        }
      );

      formatter = forAllSystems (system: nixpkgs.legacyPackages.${system}.nixfmt-rfc-style);
    };
}
