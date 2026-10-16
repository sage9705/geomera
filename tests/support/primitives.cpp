#include "primitives.hpp"

#include <cmath>
#include <cstdint>
#include <map>
#include <numbers>
#include <utility>

namespace geomera::test {

TriMesh makeTriangle() {
    TriMesh mesh;
    mesh.vertices = {
        Vec3(0.0, 0.0, 0.0),
        Vec3(1.0, 0.0, 0.0),
        Vec3(0.0, 1.0, 0.0)
    };
    mesh.faces = {
        {0, 1, 2}
    };
    return mesh;
}

TriMesh makeTetrahedron() {
    TriMesh mesh;
    mesh.vertices = {
        Vec3(1.0, 1.0, 1.0),
        Vec3(1.0, -1.0, -1.0),
        Vec3(-1.0, 1.0, -1.0),
        Vec3(-1.0, -1.0, 1.0)
    };
    mesh.faces = {
        {0, 2, 1},
        {0, 1, 3},
        {0, 3, 2},
        {1, 2, 3}
    };
    return mesh;
}

TriMesh makeBox(const Vec3& min, const Vec3& max) {
    TriMesh mesh;
    mesh.vertices = {
        Vec3(min.x(), min.y(), min.z()),
        Vec3(max.x(), min.y(), min.z()),
        Vec3(max.x(), max.y(), min.z()),
        Vec3(min.x(), max.y(), min.z()),
        Vec3(min.x(), min.y(), max.z()),
        Vec3(max.x(), min.y(), max.z()),
        Vec3(max.x(), max.y(), max.z()),
        Vec3(min.x(), max.y(), max.z())
    };
    mesh.faces = {
        {0, 2, 1}, {0, 3, 2},
        {4, 5, 6}, {4, 6, 7},
        {0, 1, 5}, {0, 5, 4},
        {2, 3, 7}, {2, 7, 6},
        {0, 4, 7}, {0, 7, 3},
        {1, 2, 6}, {1, 6, 5}
    };
    return mesh;
}

TriMesh makeIcosphere(std::size_t subdivisions, double radius) {
    TriMesh mesh;
    const double phi = (1.0 + std::sqrt(5.0)) * 0.5;

    mesh.vertices = {
        Vec3(-1.0, phi, 0.0),
        Vec3(1.0, phi, 0.0),
        Vec3(-1.0, -phi, 0.0),
        Vec3(1.0, -phi, 0.0),
        Vec3(0.0, -1.0, phi),
        Vec3(0.0, 1.0, phi),
        Vec3(0.0, -1.0, -phi),
        Vec3(0.0, 1.0, -phi),
        Vec3(phi, 0.0, -1.0),
        Vec3(phi, 0.0, 1.0),
        Vec3(-phi, 0.0, -1.0),
        Vec3(-phi, 0.0, 1.0)
    };

    for (auto& v : mesh.vertices) {
        v = v.normalized() * radius;
    }

    mesh.faces = {
        {0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
        {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
        {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1}
    };

    for (std::size_t s = 0; s < subdivisions; ++s) {
        std::map<std::pair<Index, Index>, Index> midpointCache;
        std::vector<Triangle> nextFaces;
        nextFaces.reserve(mesh.faces.size() * 4);

        auto getMidpoint = [&](Index i1, Index i2) -> Index {
            const auto key = std::minmax(i1, i2);
            if (const auto it = midpointCache.find(key); it != midpointCache.end()) {
                return it->second;
            }
            const Vec3 mid = (0.5 * (mesh.vertices[i1] + mesh.vertices[i2])).normalized() * radius;
            const Index idx = static_cast<Index>(mesh.vertices.size());
            mesh.vertices.push_back(mid);
            midpointCache[key] = idx;
            return idx;
        };

        for (const auto& f : mesh.faces) {
            const Index a = f[0];
            const Index b = f[1];
            const Index c = f[2];

            const Index ab = getMidpoint(a, b);
            const Index bc = getMidpoint(b, c);
            const Index ca = getMidpoint(c, a);

            nextFaces.push_back({a, ab, ca});
            nextFaces.push_back({b, bc, ab});
            nextFaces.push_back({c, ca, bc});
            nextFaces.push_back({ab, bc, ca});
        }

        mesh.faces = std::move(nextFaces);
    }

    return mesh;
}

TriMesh makeCylinder(double radius, double height, std::size_t segments) {
    TriMesh mesh;
    if (segments < 3) {
        segments = 3;
    }

    const double halfH = 0.5 * height;
    const double angleStep = (2.0 * std::numbers::pi) / static_cast<double>(segments);

    for (std::size_t i = 0; i < segments; ++i) {
        const double angle = static_cast<double>(i) * angleStep;
        const double x = radius * std::cos(angle);
        const double y = radius * std::sin(angle);
        mesh.vertices.emplace_back(x, y, -halfH);
    }

    for (std::size_t i = 0; i < segments; ++i) {
        const double angle = static_cast<double>(i) * angleStep;
        const double x = radius * std::cos(angle);
        const double y = radius * std::sin(angle);
        mesh.vertices.emplace_back(x, y, halfH);
    }

    const Index bottomCenter = static_cast<Index>(mesh.vertices.size());
    mesh.vertices.emplace_back(0.0, 0.0, -halfH);

    const Index topCenter = static_cast<Index>(mesh.vertices.size());
    mesh.vertices.emplace_back(0.0, 0.0, halfH);

    const auto n = static_cast<Index>(segments);

    for (Index i = 0; i < n; ++i) {
        const Index next = (i + 1) % n;
        const Index b0 = i;
        const Index b1 = next;
        const Index t0 = i + n;
        const Index t1 = next + n;

        mesh.faces.push_back({b0, b1, t1});
        mesh.faces.push_back({b0, t1, t0});

        mesh.faces.push_back({bottomCenter, b1, b0});
        mesh.faces.push_back({topCenter, t0, t1});
    }

    return mesh;
}

TriMesh makeTorus(
    double majorRadius,
    double minorRadius,
    std::size_t majorSegments,
    std::size_t minorSegments
) {
    TriMesh mesh;
    if (majorSegments < 3) {
        majorSegments = 3;
    }
    if (minorSegments < 3) {
        minorSegments = 3;
    }

    const double uStep = (2.0 * std::numbers::pi) / static_cast<double>(majorSegments);
    const double vStep = (2.0 * std::numbers::pi) / static_cast<double>(minorSegments);

    for (std::size_t i = 0; i < majorSegments; ++i) {
        const double u = static_cast<double>(i) * uStep;
        const double cosU = std::cos(u);
        const double sinU = std::sin(u);

        for (std::size_t j = 0; j < minorSegments; ++j) {
            const double v = static_cast<double>(j) * vStep;
            const double cosV = std::cos(v);
            const double sinV = std::sin(v);

            const double x = (majorRadius + minorRadius * cosV) * cosU;
            const double y = (majorRadius + minorRadius * cosV) * sinU;
            const double z = minorRadius * sinV;

            mesh.vertices.emplace_back(x, y, z);
        }
    }

    const auto nU = static_cast<Index>(majorSegments);
    const auto nV = static_cast<Index>(minorSegments);

    for (Index i = 0; i < nU; ++i) {
        const Index nextI = (i + 1) % nU;
        for (Index j = 0; j < nV; ++j) {
            const Index nextJ = (j + 1) % nV;

            const Index v00 = i * nV + j;
            const Index v10 = nextI * nV + j;
            const Index v11 = nextI * nV + nextJ;
            const Index v01 = i * nV + nextJ;

            mesh.faces.push_back({v00, v10, v11});
            mesh.faces.push_back({v00, v11, v01});
        }
    }

    return mesh;
}

}
