#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
#include <Eigen/Geometry>

namespace geomera {

using Index = std::uint32_t;
inline constexpr Index kInvalidIndex = std::numeric_limits<Index>::max();

using Vec3 = Eigen::Vector3d;
using Triangle = std::array<Index, 3>;

struct TriMesh {
    std::vector<Vec3> vertices;
    std::vector<Triangle> faces;

    [[nodiscard]] bool empty() const noexcept {
        return vertices.empty() && faces.empty();
    }

    [[nodiscard]] std::size_t vertexCount() const noexcept {
        return vertices.size();
    }

    [[nodiscard]] std::size_t faceCount() const noexcept {
        return faces.size();
    }

    void clear() noexcept {
        vertices.clear();
        faces.clear();
    }
};

struct Aabb {
    Vec3 min{
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity()
    };
    Vec3 max{
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    };

    Aabb() noexcept = default;

    Aabb(const Vec3& minPoint, const Vec3& maxPoint) noexcept
        : min(minPoint), max(maxPoint) {}

    explicit Aabb(const Vec3& point) noexcept
        : min(point), max(point) {}

    [[nodiscard]] bool isEmpty() const noexcept {
        return min.x() > max.x() || min.y() > max.y() || min.z() > max.z();
    }

    void expand(const Vec3& point) noexcept {
        min = min.cwiseMin(point);
        max = max.cwiseMax(point);
    }

    void expand(const Aabb& other) noexcept {
        if (!other.isEmpty()) {
            min = min.cwiseMin(other.min);
            max = max.cwiseMax(other.max);
        }
    }

    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        if (isEmpty()) {
            return false;
        }
        return (point.x() >= min.x() && point.x() <= max.x()) &&
               (point.y() >= min.y() && point.y() <= max.y()) &&
               (point.z() >= min.z() && point.z() <= max.z());
    }

    [[nodiscard]] bool contains(const Aabb& other) const noexcept {
        if (isEmpty() || other.isEmpty()) {
            return false;
        }
        return contains(other.min) && contains(other.max);
    }

    [[nodiscard]] Vec3 extent() const noexcept {
        if (isEmpty()) {
            return Vec3::Zero();
        }
        return max - min;
    }

    [[nodiscard]] Vec3 center() const noexcept {
        if (isEmpty()) {
            return Vec3::Zero();
        }
        return 0.5 * (min + max);
    }

    [[nodiscard]] double surfaceArea() const noexcept {
        if (isEmpty()) {
            return 0.0;
        }
        const Vec3 d = extent();
        return 2.0 * (d.x() * d.y() + d.y() * d.z() + d.z() * d.x());
    }

    [[nodiscard]] double diagonalLength() const noexcept {
        if (isEmpty()) {
            return 0.0;
        }
        return extent().norm();
    }
};

}
