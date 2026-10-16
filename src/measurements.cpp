#include <geomera/measurements.hpp>
#include <cmath>

namespace geomera {

Aabb bounds(const TriMesh& mesh) noexcept {
    Aabb box;
    for (const auto& v : mesh.vertices) {
        box.expand(v);
    }
    return box;
}

Vec3 faceNormalUnnormalized(const TriMesh& mesh, Index faceIndex) {
    if (faceIndex >= mesh.faces.size()) {
        return Vec3::Zero();
    }
    const auto& f = mesh.faces[faceIndex];
    if (f[0] >= mesh.vertices.size() || f[1] >= mesh.vertices.size() || f[2] >= mesh.vertices.size()) {
        return Vec3::Zero();
    }
    const Vec3& v0 = mesh.vertices[f[0]];
    const Vec3& v1 = mesh.vertices[f[1]];
    const Vec3& v2 = mesh.vertices[f[2]];
    return (v1 - v0).cross(v2 - v0);
}

double faceArea(const TriMesh& mesh, Index faceIndex) {
    return 0.5 * faceNormalUnnormalized(mesh, faceIndex).norm();
}

double surfaceArea(const TriMesh& mesh) noexcept {
    double total = 0.0;
    for (std::size_t i = 0; i < mesh.faces.size(); ++i) {
        total += faceArea(mesh, static_cast<Index>(i));
    }
    return total;
}

bool hasFiniteCoordinates(const TriMesh& mesh) noexcept {
    for (const auto& v : mesh.vertices) {
        if (!std::isfinite(v.x()) || !std::isfinite(v.y()) || !std::isfinite(v.z())) {
            return false;
        }
    }
    return true;
}

}
