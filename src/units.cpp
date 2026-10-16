#include <geomera/units.hpp>

#include <cctype>
#include <string>

namespace geomera {

namespace {

std::string toLowerAscii(std::string_view s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return result;
}

}

double toMillimetersScale(LengthUnit unit) noexcept {
    switch (unit) {
        case LengthUnit::Millimeter:
            return 1.0;
        case LengthUnit::Centimeter:
            return 10.0;
        case LengthUnit::Meter:
            return 1000.0;
        case LengthUnit::Inch:
            return 25.4;
        case LengthUnit::Foot:
            return 304.8;
    }
    return 1.0;
}

double fromMillimetersScale(LengthUnit unit) noexcept {
    return 1.0 / toMillimetersScale(unit);
}

Result<LengthUnit> parseLengthUnit(std::string_view unitStr) {
    const std::string lower = toLowerAscii(unitStr);
    if (lower == "mm" || lower == "millimeter" || lower == "millimeters") {
        return LengthUnit::Millimeter;
    }
    if (lower == "cm" || lower == "centimeter" || lower == "centimeters") {
        return LengthUnit::Centimeter;
    }
    if (lower == "m" || lower == "meter" || lower == "meters") {
        return LengthUnit::Meter;
    }
    if (lower == "in" || lower == "inch" || lower == "inches") {
        return LengthUnit::Inch;
    }
    if (lower == "ft" || lower == "foot" || lower == "feet") {
        return LengthUnit::Foot;
    }
    return tl::make_unexpected(Error{
        ErrorCode::OperationFailed,
        "Unknown length unit: " + std::string(unitStr)
    });
}

std::string_view lengthUnitToString(LengthUnit unit) noexcept {
    switch (unit) {
        case LengthUnit::Millimeter:
            return "mm";
        case LengthUnit::Centimeter:
            return "cm";
        case LengthUnit::Meter:
            return "m";
        case LengthUnit::Inch:
            return "in";
        case LengthUnit::Foot:
            return "ft";
    }
    return "mm";
}

void scaleMesh(TriMesh& mesh, double scaleFactor) noexcept {
    for (auto& vertex : mesh.vertices) {
        vertex *= scaleFactor;
    }
}

}
