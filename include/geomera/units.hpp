#pragma once

#include <geomera/error.hpp>
#include <geomera/mesh.hpp>

#include <string_view>

namespace geomera {

enum class LengthUnit {
    Millimeter,
    Centimeter,
    Meter,
    Inch,
    Foot
};

[[nodiscard]] double toMillimetersScale(LengthUnit unit) noexcept;
[[nodiscard]] double fromMillimetersScale(LengthUnit unit) noexcept;

[[nodiscard]] Result<LengthUnit> parseLengthUnit(std::string_view unitStr);
[[nodiscard]] std::string_view lengthUnitToString(LengthUnit unit) noexcept;

void scaleMesh(TriMesh& mesh, double scaleFactor) noexcept;

}
