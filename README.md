# Geomera

A computational geometry toolkit for triangle mesh processing, repair, registration, and simplification. C++20, double precision, vcpkg.

## Status

Phases 0–1 complete. Phase 2 (file I/O) in progress — STL reader/writer done, PLY and OBJ next.

## Build

Requires C++20, CMake 3.21+, Ninja, and vcpkg (`VCPKG_ROOT` must be set).

```bash
cmake --preset dev
cmake --build build/dev
ctest --test-dir build/dev --output-on-failure
```

Presets: `dev` (debug + compile_commands), `release`, `asan`, `ci`.

## What exists so far

**Core types** — `TriMesh` (indexed triangle mesh), `Vec3` (Eigen::Vector3d), `Aabb`, `Index` with `kInvalidIndex` sentinel.

**Measurements** — `bounds`, `faceNormalUnnormalized`, `faceArea`, `surfaceArea`, `hasFiniteCoordinates`.

**Error handling** — `Result<T>` via `tl::expected`, structured `Error` with stable codes, `Warning`, `OpOutput<T>`. JSON-serializable.

**Tolerances** — centralized `Tolerances` struct with named defaults, units (mm, mm², rad), and validity checks.

**STL I/O** — binary and ASCII reader with automatic format detection, exact-bit vertex welding (negative-zero normalization), truncation/corruption detection. Binary and ASCII writer.

**Test fixtures** — deterministic primitive generators (triangle, box, tetrahedron, icosphere, cylinder, torus).

## Dependencies

Eigen3, GoogleTest, Google Benchmark, CLI11, nlohmann/json, spdlog, tl-expected. Managed via vcpkg manifest with pinned baseline.

## CI

GCC + Clang (Debug/Release), ASan/UBSan, clang-tidy (advisory). All on GitHub Actions.

## License

TBD
