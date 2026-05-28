# val3dity wasm MVP

This is a thin Emscripten/Embind wrapper around the existing `val3dity` C++ API.

It exposes three validation entry points:

- `validateCityJSON(input, options?)`, where `input` is a CityJSON string.
- `validateCityJSONSeq(input, options?)`, where `input` is a CityJSONSeq / JSON Lines string.
- `validateRawArrays(vertices, faces, options?)`, where `vertices` is either a flat xyz array or an array of `[x, y, z]` triples, and `faces` is either `[[0, 1, 2, 3], ...]` or `[[[outer], [hole]], ...]`.

All functions return the normal val3dity report as a JavaScript object.

```js
import { createVal3dity } from "./val3dity.js";

const val3dity = await createVal3dity();

const report = val3dity.validateRawArrays(
  [
    0, 0, 0,
    1, 0, 0,
    1, 1, 0,
    0, 1, 0,
    0, 0, 1,
    1, 0, 1,
    1, 1, 1,
    0, 1, 1,
  ],
  [
    [0, 3, 2, 1],
    [4, 5, 6, 7],
    [0, 1, 5, 4],
    [1, 2, 6, 5],
    [2, 3, 7, 6],
    [3, 0, 4, 7],
  ],
  { primitive: "Solid" },
);

console.log(report.validity);
```

Build from the wasm dev shell:

```sh
nix develop .#wasm
wasm-build
```

The build writes `build-wasm/val3dity_wasm.mjs` and `build-wasm/val3dity_wasm.wasm`.

The same directory also contains `demo.html`, a small browser demo for uploading a CityJSON or CityJSONSeq file and viewing the validation report. Serve the directory over HTTP, for example:

```sh
python3 -m http.server -d build-wasm 8000
```

Then open `http://localhost:8000/demo.html`.

## Caveats

The wasm reports can differ from reports produced by the native `val3dity` executable. Some validation steps depend on best-fit-plane projection, constrained triangulation and CGAL polygon-mesh predicates, so borderline geometries can produce different results.

Known local findings:

- `Ingolstadt.city.json` (Cityjson.org example dataset): native reports `102`, `104` and `204` errors only. The wasm builds tested also report `999` errors such as `face does not have an outer boundary`, and can report fewer `204` `NON_PLANAR_POLYGON_NORMALS_DEVIATION` errors because failed triangulation prevents later checks from running.
- `10-434-716.city.jsonl` (3DBAG): native reports 24 `306` self-intersection errors, while wasm reports 2205 `306` errors and one `999`. For one inspected object, native and wasm built the same triangle mesh, but `CGAL::Polygon_mesh_processing::self_intersections()` reported extra intersecting triangle pairs only in wasm. The reported `306` location is the centroid of a flagged triangle, not the exact intersection point, so it can look displaced from the apparent intersection.

GMP/MPFR are not impossible on wasm, but they are not part of this build. They can be cross-compiled with Emscripten as wasm static libraries using generic C / disabled assembly paths, and GMP can also be built with `--enable-cxx` to provide `libgmpxx.a`. CGAL can then be configured to find those wasm headers and libraries, remove `CGAL_DISABLE_GMP=ON`, and select either `GMP_BACKEND` or `GMPXX_BACKEND`. Expect a larger wasm artifact and slower exact-number operations.

The current discrepancies do not appear to be explained only by GMP being absent. Local tests showed:

- native default, native `CGAL_DISABLE_GMP=ON`, and native `CGAL_CMAKE_EXACT_NT_BACKEND=BOOST_BACKEND` produced the same reports for the two samples above;
- wasm `GMP_BACKEND`, wasm `GMPXX_BACKEND`, wasm `BOOST_BACKEND`, and wasm with `-frounding-math` still reproduced the observed wasm discrepancies.

The remaining suspect is wasm/Emscripten-specific behaviour in CGAL's geometry predicates or triangulation/intersection paths, rather than a simple GMP-vs-no-GMP difference.

Also be careful when reusing one wasm module for many validations in the same page. Some CityJSON parsing state is process-global in the native code path, so repeated validations in a long-lived wasm instance may differ from running the CLI once per file in a fresh process.

Supported options:

- `tolSnap` or `tol_snap`
- `planarityD2pTol` or `planarity_d2p_tol`
- `planarityNTol` or `planarity_n_tol`
- `overlapTol` or `overlap_tol`
- `primitive`: `Solid`, `MultiSurface`, or `CompositeSurface`
