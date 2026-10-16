#pragma once

#include <cstddef>
#include <geomera/mesh.hpp>

namespace geomera::test {

[[nodiscard]] TriMesh makeTriangle();

[[nodiscard]] TriMesh makeTetrahedron();

[[nodiscard]] TriMesh makeBox(
    const Vec3& min = Vec3(-1.0, -1.0, -1.0),
    const Vec3& max = Vec3(1.0, 1.0, 1.0)
);

[[nodiscard]] TriMesh makeIcosphere(
    std::size_t subdivisions = 0,
    double radius = 1.0
);

[[nodiscard]] TriMesh makeCylinder(
    double radius = 1.0,
    double height = 2.0,
    std::size_t segments = 16
);

[[nodiscard]] TriMesh makeTorus(
    double majorRadius = 2.0,
    double minorRadius = 0.5,
    std::size_t majorSegments = 16,
    std::size_t minorSegments = 8
);

}
