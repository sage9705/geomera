#pragma once

#include <geomera/mesh.hpp>

namespace geomera {

[[nodiscard]] Aabb bounds(const TriMesh& mesh) noexcept;

[[nodiscard]] Vec3 faceNormalUnnormalized(const TriMesh& mesh, Index faceIndex);

[[nodiscard]] double faceArea(const TriMesh& mesh, Index faceIndex);

[[nodiscard]] double surfaceArea(const TriMesh& mesh) noexcept;

[[nodiscard]] bool hasFiniteCoordinates(const TriMesh& mesh) noexcept;

}
