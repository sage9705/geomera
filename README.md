# Geomera

A computational geometry toolkit for triangle mesh processing, validation, repair, registration, and simplification. Implemented in C++20 using double-precision floating point coordinates.

## Building and Running

### Prerequisites

- C++20 compatible compiler (MSVC 19.34+, GCC 11+, or Clang 13+)
- CMake 3.21+
- Ninja build system
- vcpkg package manager with `VCPKG_ROOT` set

### Configure and Build

Configure with the desired preset (`dev`, `release`, `asan`, or `ci`):

```bash
cmake --preset dev
```

Build all targets using Ninja:

```bash
ninja -C build/dev
```

### Running Tests

Run the test suite using Ninja:

```bash
ninja -C build/dev test
```

Or via CTest with failure output:

```bash
ctest --test-dir build/dev --output-on-failure
```

Run specific test filters directly:

```bash
# Windows
build\dev\geomera_tests.exe --gtest_filter=ObjTest.*

# Linux / macOS
./build/dev/geomera_tests --gtest_filter=ObjTest.*
```

### Running the CLI and Benchmarks

Run the CLI tool:

```bash
# Windows
build\dev\meshtool.exe --help

# Linux / macOS
./build/dev/meshtool --help
```

Run microbenchmarks:

```bash
# Windows
build\dev\geomera_bench.exe

# Linux / macOS
./build/dev/geomera_bench
```

## Features

### Core Data Model
- `TriMesh`: Indexed triangle mesh structure storing vertices (`std::vector<Vec3>`) and triangular faces (`std::vector<Triangle>`).
- `Vec3`: 3D double-precision vectors using `Eigen::Vector3d`.
- `Aabb`: 3D axis-aligned bounding box with expansion, intersection, containment tests, surface area, and diagonal queries.
- `Index`: `std::uint32_t` indexing with explicit `kInvalidIndex` sentinel.

### Measurements
- Bounding box calculation (`bounds`).
- Face area and cumulative mesh surface area (`faceArea`, `surfaceArea`).
- Face normal calculation (`faceNormalUnnormalized`).
- Finite coordinate validation (`hasFiniteCoordinates`).

### File Formats
- **STL** (`stl.hpp`): Binary and ASCII reader with format auto-detection, exact-bit vertex welding, negative-zero normalization, and corruption checking. Binary and ASCII writer.
- **PLY** (`ply.hpp`): Reader supporting ASCII, binary little-endian, and binary big-endian formats. Reads vertex coordinates, skips arbitrary unused properties, and fan-triangulates polygonal faces.
- **OBJ** (`obj.hpp`): Wavefront OBJ reader supporting `v`, `v/vt`, `v//vn`, and `v/vt/vn` face declarations, negative relative indexing, and polygon fan-triangulation. Safely ignores comments, materials, and groups.

### Errors and Diagnostics
- Structured `Error` types with stable codes (`ErrorCode`), messages, and JSON details.
- Monadic `Result<T>` error handling based on `tl::expected`.
- `Warning` diagnostic records for non-fatal modifications (such as polygon fan-triangulation).
- `OpOutput<T>` packaging results alongside warnings and runtime metrics (execution duration, memory estimation).

### Tolerances
- Centralized `Tolerances` configuration defining geometric thresholds (coincident vertices, degenerate area, collinear angle, non-manifold distance) in explicit units (mm, mm², radians) with parameter validation.

### Synthetic Test Fixtures
- Deterministic primitive mesh generators for testing: triangle, tetrahedron, box, icosphere, cylinder, and torus.

## Dependencies

- Eigen3
- GoogleTest
- Google Benchmark
- CLI11
- nlohmann/json
- spdlog
- tl-expected
