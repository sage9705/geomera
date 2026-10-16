#include <geomera/ply.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace geomera {

namespace {

enum class PlyScalarType { Char, UChar, Short, UShort, Int, UInt, Float, Double };

[[nodiscard]] std::size_t scalarSize(PlyScalarType type) noexcept {
    switch (type) {
        case PlyScalarType::Char:
        case PlyScalarType::UChar:
            return 1;
        case PlyScalarType::Short:
        case PlyScalarType::UShort:
            return 2;
        case PlyScalarType::Int:
        case PlyScalarType::UInt:
        case PlyScalarType::Float:
            return 4;
        case PlyScalarType::Double:
            return 8;
    }
    return 1;
}

[[nodiscard]] Result<PlyScalarType> parseScalarType(std::string_view name) {
    if (name == "char" || name == "int8") {
        return PlyScalarType::Char;
    }
    if (name == "uchar" || name == "uint8") {
        return PlyScalarType::UChar;
    }
    if (name == "short" || name == "int16") {
        return PlyScalarType::Short;
    }
    if (name == "ushort" || name == "uint16") {
        return PlyScalarType::UShort;
    }
    if (name == "int" || name == "int32") {
        return PlyScalarType::Int;
    }
    if (name == "uint" || name == "uint32") {
        return PlyScalarType::UInt;
    }
    if (name == "float" || name == "float32") {
        return PlyScalarType::Float;
    }
    if (name == "double" || name == "float64") {
        return PlyScalarType::Double;
    }
    return tl::make_unexpected(
        Error{ErrorCode::IoError, "Unsupported PLY property type: " + std::string(name)});
}

struct PlyProperty {
    std::string name;
    bool isList{false};
    PlyScalarType scalarType{PlyScalarType::Float};
    PlyScalarType listCountType{PlyScalarType::UChar};
    PlyScalarType listItemType{PlyScalarType::Int};
};

struct PlyElement {
    std::string name;
    std::size_t count{0};
    std::vector<PlyProperty> properties;
};

struct PlyHeader {
    PlyFormat format{PlyFormat::Ascii};
    std::vector<PlyElement> elements;
    std::size_t headerByteSize{0};
};

[[nodiscard]] uint16_t swap16(uint16_t v) noexcept {
    return static_cast<uint16_t>((v << 8) | (v >> 8));
}

[[nodiscard]] uint32_t swap32(uint32_t v) noexcept {
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) | ((v & 0x00FF0000u) >> 8) |
           ((v & 0xFF000000u) >> 24);
}

[[nodiscard]] uint64_t swap64(uint64_t v) noexcept {
    return ((v & 0x00000000000000FFULL) << 56) | ((v & 0x000000000000FF00ULL) << 40) |
           ((v & 0x0000000000FF0000ULL) << 24) | ((v & 0x00000000FF000000ULL) << 8) |
           ((v & 0x000000FF00000000ULL) >> 8) | ((v & 0x0000FF0000000000ULL) >> 24) |
           ((v & 0x00FF000000000000ULL) >> 40) | ((v & 0xFF00000000000000ULL) >> 56);
}

template <typename T>
[[nodiscard]] T readEndian(const uint8_t* ptr, bool isBigEndian) noexcept {
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    if constexpr (sizeof(T) == 1) {
        T val;
        std::memcpy(&val, ptr, 1);
        return val;
    } else if constexpr (sizeof(T) == 2) {
        uint16_t u;
        std::memcpy(&u, ptr, 2);
        if (isBigEndian) {
            u = swap16(u);
        }
        T val;
        std::memcpy(&val, &u, 2);
        return val;
    } else if constexpr (sizeof(T) == 4) {
        uint32_t u;
        std::memcpy(&u, ptr, 4);
        if (isBigEndian) {
            u = swap32(u);
        }
        T val;
        std::memcpy(&val, &u, 4);
        return val;
    } else {
        uint64_t u;
        std::memcpy(&u, ptr, 8);
        if (isBigEndian) {
            u = swap64(u);
        }
        T val;
        std::memcpy(&val, &u, 8);
        return val;
    }
}

[[nodiscard]] double readScalarAsDouble(const uint8_t* ptr,
                                        PlyScalarType type,
                                        bool isBigEndian) noexcept {
    switch (type) {
        case PlyScalarType::Char:
            return static_cast<double>(readEndian<int8_t>(ptr, isBigEndian));
        case PlyScalarType::UChar:
            return static_cast<double>(readEndian<uint8_t>(ptr, isBigEndian));
        case PlyScalarType::Short:
            return static_cast<double>(readEndian<int16_t>(ptr, isBigEndian));
        case PlyScalarType::UShort:
            return static_cast<double>(readEndian<uint16_t>(ptr, isBigEndian));
        case PlyScalarType::Int:
            return static_cast<double>(readEndian<int32_t>(ptr, isBigEndian));
        case PlyScalarType::UInt:
            return static_cast<double>(readEndian<uint32_t>(ptr, isBigEndian));
        case PlyScalarType::Float:
            return static_cast<double>(readEndian<float>(ptr, isBigEndian));
        case PlyScalarType::Double:
            return readEndian<double>(ptr, isBigEndian);
    }
    return 0.0;
}

[[nodiscard]] int64_t readScalarAsInt64(const uint8_t* ptr,
                                        PlyScalarType type,
                                        bool isBigEndian) noexcept {
    switch (type) {
        case PlyScalarType::Char:
            return static_cast<int64_t>(readEndian<int8_t>(ptr, isBigEndian));
        case PlyScalarType::UChar:
            return static_cast<int64_t>(readEndian<uint8_t>(ptr, isBigEndian));
        case PlyScalarType::Short:
            return static_cast<int64_t>(readEndian<int16_t>(ptr, isBigEndian));
        case PlyScalarType::UShort:
            return static_cast<int64_t>(readEndian<uint16_t>(ptr, isBigEndian));
        case PlyScalarType::Int:
            return static_cast<int64_t>(readEndian<int32_t>(ptr, isBigEndian));
        case PlyScalarType::UInt:
            return static_cast<int64_t>(readEndian<uint32_t>(ptr, isBigEndian));
        case PlyScalarType::Float:
            return static_cast<int64_t>(readEndian<float>(ptr, isBigEndian));
        case PlyScalarType::Double:
            return static_cast<int64_t>(readEndian<double>(ptr, isBigEndian));
    }
    return 0;
}

Result<PlyHeader> parseHeader(const std::vector<uint8_t>& buf) {
    std::string headerStr;
    const std::string_view magic = "ply";
    if (buf.size() < 3 || std::string_view(reinterpret_cast<const char*>(buf.data()), 3) != magic) {
        return tl::make_unexpected(
            Error{ErrorCode::IoError, "Not a valid PLY file: magic 'ply' missing at start"});
    }

    std::size_t endHeaderPos = std::string::npos;
    std::size_t endHeaderLen = 0;
    const std::string_view fullBuf(reinterpret_cast<const char*>(buf.data()), buf.size());

    const std::string marker = "end_header";
    auto pos = fullBuf.find(marker);
    if (pos == std::string_view::npos) {
        return tl::make_unexpected(
            Error{ErrorCode::IoError, "PLY header missing 'end_header' marker"});
    }

    std::size_t newlinePos = pos + marker.size();
    if (newlinePos < buf.size() && fullBuf[newlinePos] == '\r') {
        ++newlinePos;
    }
    if (newlinePos < buf.size() && fullBuf[newlinePos] == '\n') {
        ++newlinePos;
    }
    endHeaderPos = pos;
    endHeaderLen = newlinePos;

    headerStr = std::string(fullBuf.substr(0, endHeaderPos));
    std::istringstream stream(headerStr);
    std::string line;

    if (!std::getline(stream, line)) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "Empty PLY header"});
    }

    PlyHeader header;
    header.headerByteSize = endHeaderLen;
    bool hasFormat = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::istringstream lineStream(line);
        std::string token;
        if (!(lineStream >> token)) {
            continue;
        }

        if (token == "comment" || token == "obj_info") {
            continue;
        }

        if (token == "format") {
            std::string formatName;
            std::string version;
            if (!(lineStream >> formatName >> version)) {
                return tl::make_unexpected(
                    Error{ErrorCode::IoError, "Malformed format line in PLY header"});
            }
            if (formatName == "ascii") {
                header.format = PlyFormat::Ascii;
            } else if (formatName == "binary_little_endian") {
                header.format = PlyFormat::BinaryLittleEndian;
            } else if (formatName == "binary_big_endian") {
                header.format = PlyFormat::BinaryBigEndian;
            } else {
                return tl::make_unexpected(
                    Error{ErrorCode::IoError, "Unsupported PLY format: " + formatName});
            }
            hasFormat = true;
        } else if (token == "element") {
            std::string elemName;
            std::size_t count = 0;
            if (!(lineStream >> elemName >> count)) {
                return tl::make_unexpected(
                    Error{ErrorCode::IoError, "Malformed element declaration in PLY header"});
            }
            header.elements.push_back(PlyElement{elemName, count, {}});
        } else if (token == "property") {
            if (header.elements.empty()) {
                return tl::make_unexpected(
                    Error{ErrorCode::IoError, "Property defined before any element in PLY header"});
            }
            std::string typeOrList;
            if (!(lineStream >> typeOrList)) {
                return tl::make_unexpected(
                    Error{ErrorCode::IoError, "Malformed property declaration in PLY header"});
            }

            PlyProperty prop;
            if (typeOrList == "list") {
                std::string countTypeStr;
                std::string itemTypeStr;
                std::string propName;
                if (!(lineStream >> countTypeStr >> itemTypeStr >> propName)) {
                    return tl::make_unexpected(Error{
                        ErrorCode::IoError, "Malformed list property declaration in PLY header"});
                }
                auto cType = parseScalarType(countTypeStr);
                if (!cType) {
                    return tl::make_unexpected(cType.error());
                }
                auto iType = parseScalarType(itemTypeStr);
                if (!iType) {
                    return tl::make_unexpected(iType.error());
                }
                prop.isList = true;
                prop.name = propName;
                prop.listCountType = *cType;
                prop.listItemType = *iType;
            } else {
                std::string propName;
                if (!(lineStream >> propName)) {
                    return tl::make_unexpected(Error{
                        ErrorCode::IoError, "Malformed scalar property declaration in PLY header"});
                }
                auto sType = parseScalarType(typeOrList);
                if (!sType) {
                    return tl::make_unexpected(sType.error());
                }
                prop.isList = false;
                prop.name = propName;
                prop.scalarType = *sType;
            }
            header.elements.back().properties.push_back(prop);
        }
    }

    if (!hasFormat) {
        return tl::make_unexpected(
            Error{ErrorCode::IoError, "PLY header missing format specification"});
    }

    return header;
}

Result<OpOutput<TriMesh>> parseAsciiPly(const uint8_t* data,
                                        std::size_t size,
                                        const PlyHeader& header) {
    std::string text(reinterpret_cast<const char*>(data), size);
    std::istringstream stream(text);

    TriMesh mesh;
    std::vector<Warning> warnings;

    const PlyElement* vertexElem = nullptr;
    const PlyElement* faceElem = nullptr;

    for (const auto& elem : header.elements) {
        if (elem.name == "vertex" && !vertexElem) {
            vertexElem = &elem;
        } else if (elem.name == "face" && !faceElem) {
            faceElem = &elem;
        }
    }

    if (!vertexElem) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "PLY missing 'vertex' element"});
    }

    std::size_t xIndex = std::string::npos;
    std::size_t yIndex = std::string::npos;
    std::size_t zIndex = std::string::npos;

    for (std::size_t i = 0; i < vertexElem->properties.size(); ++i) {
        const auto& prop = vertexElem->properties[i];
        if (prop.name == "x")
            xIndex = i;
        else if (prop.name == "y")
            yIndex = i;
        else if (prop.name == "z")
            zIndex = i;
    }

    if (xIndex == std::string::npos || yIndex == std::string::npos || zIndex == std::string::npos) {
        return tl::make_unexpected(
            Error{ErrorCode::InvalidMesh, "PLY vertex element missing x, y, or z property"});
    }

    mesh.vertices.reserve(vertexElem->count);

    for (const auto& elem : header.elements) {
        if (elem.name == "vertex") {
            for (std::size_t i = 0; i < elem.count; ++i) {
                double x = 0.0;
                double y = 0.0;
                double z = 0.0;
                for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                    if (elem.properties[p].isList) {
                        std::size_t listCount = 0;
                        if (!(stream >> listCount)) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError, "Truncated ASCII vertex list property"});
                        }
                        for (std::size_t k = 0; k < listCount; ++k) {
                            std::string dummy;
                            if (!(stream >> dummy)) {
                                return tl::make_unexpected(
                                    Error{ErrorCode::IoError, "Truncated ASCII vertex list item"});
                            }
                        }
                    } else {
                        std::string token;
                        if (!(stream >> token)) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated ASCII vertex coordinate at index " +
                                          std::to_string(i)});
                        }
                        double val = 0.0;
                        try {
                            val = std::stod(token);
                        } catch (...) {
                            return tl::make_unexpected(
                                Error{ErrorCode::InvalidMesh,
                                      "Malformed ASCII vertex coordinate at index " +
                                          std::to_string(i)});
                        }
                        if (p == xIndex)
                            x = val;
                        else if (p == yIndex)
                            y = val;
                        else if (p == zIndex)
                            z = val;
                    }
                }

                if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
                    return tl::make_unexpected(
                        Error{ErrorCode::InvalidMesh,
                              "Non-finite vertex coordinate in PLY at index " + std::to_string(i)});
                }
                mesh.vertices.emplace_back(x, y, z);
            }
        } else if (elem.name == "face") {
            std::size_t faceListPropIdx = std::string::npos;
            for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                if (elem.properties[p].isList) {
                    faceListPropIdx = p;
                    break;
                }
            }

            if (faceListPropIdx == std::string::npos && elem.count > 0) {
                return tl::make_unexpected(Error{
                    ErrorCode::IoError, "PLY face element missing vertex indices list property"});
            }

            for (std::size_t i = 0; i < elem.count; ++i) {
                std::vector<Index> faceVertices;
                for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                    if (elem.properties[p].isList) {
                        std::size_t listCount = 0;
                        if (!(stream >> listCount)) {
                            return tl::make_unexpected(Error{
                                ErrorCode::IoError,
                                "Truncated ASCII face list count at face " + std::to_string(i)});
                        }
                        std::vector<int64_t> items(listCount);
                        for (std::size_t k = 0; k < listCount; ++k) {
                            if (!(stream >> items[k])) {
                                return tl::make_unexpected(
                                    Error{ErrorCode::IoError,
                                          "Truncated ASCII face vertex index at face " +
                                              std::to_string(i)});
                            }
                        }
                        if (p == faceListPropIdx) {
                            if (listCount < 3) {
                                return tl::make_unexpected(Error{
                                    ErrorCode::InvalidMesh,
                                    "Face " + std::to_string(i) + " has fewer than 3 vertices"});
                            }
                            faceVertices.reserve(listCount);
                            for (int64_t idx : items) {
                                if (idx < 0 ||
                                    static_cast<std::size_t>(idx) >= mesh.vertices.size()) {
                                    return tl::make_unexpected(
                                        Error{ErrorCode::InvalidMesh,
                                              "Out-of-bounds vertex index " + std::to_string(idx) +
                                                  " in face " + std::to_string(i)});
                                }
                                faceVertices.push_back(static_cast<Index>(idx));
                            }
                        }
                    } else {
                        std::string dummy;
                        if (!(stream >> dummy)) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError, "Truncated ASCII face extra property"});
                        }
                    }
                }

                if (!faceVertices.empty()) {
                    if (faceVertices.size() > 3) {
                        warnings.push_back(Warning{"PLY_POLYGON_TRIANGULATED",
                                                   "Face with " +
                                                       std::to_string(faceVertices.size()) +
                                                       " vertices fan-triangulated",
                                                   {{"vertex_count", faceVertices.size()}}});
                    }
                    for (std::size_t k = 1; k + 1 < faceVertices.size(); ++k) {
                        mesh.faces.push_back(
                            Triangle{faceVertices[0], faceVertices[k], faceVertices[k + 1]});
                    }
                }
            }
        } else {
            for (std::size_t i = 0; i < elem.count; ++i) {
                for (const auto& prop : elem.properties) {
                    if (prop.isList) {
                        std::size_t listCount = 0;
                        if (!(stream >> listCount)) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError, "Truncated ASCII custom element list"});
                        }
                        for (std::size_t k = 0; k < listCount; ++k) {
                            std::string dummy;
                            if (!(stream >> dummy)) {
                                return tl::make_unexpected(
                                    Error{ErrorCode::IoError,
                                          "Truncated ASCII custom element list item"});
                            }
                        }
                    } else {
                        std::string dummy;
                        if (!(stream >> dummy)) {
                            return tl::make_unexpected(Error{
                                ErrorCode::IoError, "Truncated ASCII custom element property"});
                        }
                    }
                }
            }
        }
    }

    OpOutput<TriMesh> result;
    result.value = std::move(mesh);
    result.warnings = std::move(warnings);
    return result;
}

Result<OpOutput<TriMesh>> parseBinaryPly(const uint8_t* data,
                                         std::size_t size,
                                         const PlyHeader& header,
                                         bool isBigEndian) {
    TriMesh mesh;
    std::vector<Warning> warnings;

    const PlyElement* vertexElem = nullptr;
    const PlyElement* faceElem = nullptr;

    for (const auto& elem : header.elements) {
        if (elem.name == "vertex" && !vertexElem) {
            vertexElem = &elem;
        } else if (elem.name == "face" && !faceElem) {
            faceElem = &elem;
        }
    }

    if (!vertexElem) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "PLY missing 'vertex' element"});
    }

    std::size_t xIndex = std::string::npos;
    std::size_t yIndex = std::string::npos;
    std::size_t zIndex = std::string::npos;

    for (std::size_t i = 0; i < vertexElem->properties.size(); ++i) {
        const auto& prop = vertexElem->properties[i];
        if (prop.name == "x")
            xIndex = i;
        else if (prop.name == "y")
            yIndex = i;
        else if (prop.name == "z")
            zIndex = i;
    }

    if (xIndex == std::string::npos || yIndex == std::string::npos || zIndex == std::string::npos) {
        return tl::make_unexpected(
            Error{ErrorCode::InvalidMesh, "PLY vertex element missing x, y, or z property"});
    }

    mesh.vertices.reserve(vertexElem->count);
    std::size_t offset = 0;

    for (const auto& elem : header.elements) {
        if (elem.name == "vertex") {
            for (std::size_t i = 0; i < elem.count; ++i) {
                double x = 0.0;
                double y = 0.0;
                double z = 0.0;

                for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                    const auto& prop = elem.properties[p];
                    if (prop.isList) {
                        const std::size_t cSize = scalarSize(prop.listCountType);
                        if (offset + cSize > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY reading vertex list count"});
                        }
                        const int64_t count =
                            readScalarAsInt64(data + offset, prop.listCountType, isBigEndian);
                        offset += cSize;
                        if (count < 0) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError, "Negative list count in binary PLY"});
                        }
                        const std::size_t itemSize = scalarSize(prop.listItemType);
                        const std::size_t totalBytes = static_cast<std::size_t>(count) * itemSize;
                        if (offset + totalBytes > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY reading vertex list items"});
                        }
                        offset += totalBytes;
                    } else {
                        const std::size_t sSize = scalarSize(prop.scalarType);
                        if (offset + sSize > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY reading vertex property"});
                        }
                        const double val =
                            readScalarAsDouble(data + offset, prop.scalarType, isBigEndian);
                        offset += sSize;
                        if (p == xIndex)
                            x = val;
                        else if (p == yIndex)
                            y = val;
                        else if (p == zIndex)
                            z = val;
                    }
                }

                if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
                    return tl::make_unexpected(
                        Error{ErrorCode::InvalidMesh,
                              "Non-finite vertex coordinate in binary PLY at index " +
                                  std::to_string(i)});
                }
                mesh.vertices.emplace_back(x, y, z);
            }
        } else if (elem.name == "face") {
            std::size_t faceListPropIdx = std::string::npos;
            for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                if (elem.properties[p].isList) {
                    faceListPropIdx = p;
                    break;
                }
            }

            if (faceListPropIdx == std::string::npos && elem.count > 0) {
                return tl::make_unexpected(Error{
                    ErrorCode::IoError, "PLY face element missing vertex indices list property"});
            }

            for (std::size_t i = 0; i < elem.count; ++i) {
                std::vector<Index> faceVertices;
                for (std::size_t p = 0; p < elem.properties.size(); ++p) {
                    const auto& prop = elem.properties[p];
                    if (prop.isList) {
                        const std::size_t cSize = scalarSize(prop.listCountType);
                        if (offset + cSize > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY reading face list count"});
                        }
                        const int64_t count =
                            readScalarAsInt64(data + offset, prop.listCountType, isBigEndian);
                        offset += cSize;
                        if (count < 0) {
                            return tl::make_unexpected(Error{
                                ErrorCode::IoError, "Negative face vertex count in binary PLY"});
                        }

                        const std::size_t itemSize = scalarSize(prop.listItemType);
                        if (p == faceListPropIdx) {
                            if (count < 3) {
                                return tl::make_unexpected(Error{
                                    ErrorCode::InvalidMesh,
                                    "Face " + std::to_string(i) + " has fewer than 3 vertices"});
                            }
                            faceVertices.reserve(static_cast<std::size_t>(count));
                            for (int64_t k = 0; k < count; ++k) {
                                if (offset + itemSize > size) {
                                    return tl::make_unexpected(
                                        Error{ErrorCode::IoError,
                                              "Truncated binary PLY reading face vertex index"});
                                }
                                const int64_t idx = readScalarAsInt64(
                                    data + offset, prop.listItemType, isBigEndian);
                                offset += itemSize;

                                if (idx < 0 ||
                                    static_cast<std::size_t>(idx) >= mesh.vertices.size()) {
                                    return tl::make_unexpected(
                                        Error{ErrorCode::InvalidMesh,
                                              "Out-of-bounds vertex index " + std::to_string(idx) +
                                                  " in binary PLY face " + std::to_string(i)});
                                }
                                faceVertices.push_back(static_cast<Index>(idx));
                            }
                        } else {
                            const std::size_t totalBytes =
                                static_cast<std::size_t>(count) * itemSize;
                            if (offset + totalBytes > size) {
                                return tl::make_unexpected(
                                    Error{ErrorCode::IoError,
                                          "Truncated binary PLY skipping extra face list items"});
                            }
                            offset += totalBytes;
                        }
                    } else {
                        const std::size_t sSize = scalarSize(prop.scalarType);
                        if (offset + sSize > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY reading extra face property"});
                        }
                        offset += sSize;
                    }
                }

                if (!faceVertices.empty()) {
                    if (faceVertices.size() > 3) {
                        warnings.push_back(Warning{"PLY_POLYGON_TRIANGULATED",
                                                   "Face with " +
                                                       std::to_string(faceVertices.size()) +
                                                       " vertices fan-triangulated",
                                                   {{"vertex_count", faceVertices.size()}}});
                    }
                    for (std::size_t k = 1; k + 1 < faceVertices.size(); ++k) {
                        mesh.faces.push_back(
                            Triangle{faceVertices[0], faceVertices[k], faceVertices[k + 1]});
                    }
                }
            }
        } else {
            for (std::size_t i = 0; i < elem.count; ++i) {
                for (const auto& prop : elem.properties) {
                    if (prop.isList) {
                        const std::size_t cSize = scalarSize(prop.listCountType);
                        if (offset + cSize > size) {
                            return tl::make_unexpected(Error{
                                ErrorCode::IoError, "Truncated binary PLY custom element list"});
                        }
                        const int64_t count =
                            readScalarAsInt64(data + offset, prop.listCountType, isBigEndian);
                        offset += cSize;
                        if (count < 0) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError, "Negative list count in binary PLY"});
                        }
                        const std::size_t itemSize = scalarSize(prop.listItemType);
                        const std::size_t totalBytes = static_cast<std::size_t>(count) * itemSize;
                        if (offset + totalBytes > size) {
                            return tl::make_unexpected(Error{
                                ErrorCode::IoError, "Truncated binary PLY custom element items"});
                        }
                        offset += totalBytes;
                    } else {
                        const std::size_t sSize = scalarSize(prop.scalarType);
                        if (offset + sSize > size) {
                            return tl::make_unexpected(
                                Error{ErrorCode::IoError,
                                      "Truncated binary PLY custom element property"});
                        }
                        offset += sSize;
                    }
                }
            }
        }
    }

    OpOutput<TriMesh> result;
    result.value = std::move(mesh);
    result.warnings = std::move(warnings);
    return result;
}

}  // namespace

Result<OpOutput<TriMesh>> readPlyWithReport(std::istream& is) {
    const auto startTime = std::chrono::steady_clock::now();

    std::vector<uint8_t> buffer;
    buffer.reserve(4096);
    char chunk[4096];
    while (is.read(chunk, sizeof(chunk))) {
        buffer.insert(buffer.end(),
                      reinterpret_cast<uint8_t*>(chunk),
                      reinterpret_cast<uint8_t*>(chunk) + is.gcount());
    }
    if (is.gcount() > 0) {
        buffer.insert(buffer.end(),
                      reinterpret_cast<uint8_t*>(chunk),
                      reinterpret_cast<uint8_t*>(chunk) + is.gcount());
    }

    if (buffer.empty()) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "PLY input is empty"});
    }

    auto headerRes = parseHeader(buffer);
    if (!headerRes) {
        return tl::make_unexpected(headerRes.error());
    }

    const auto& header = *headerRes;
    const uint8_t* payload = buffer.data() + header.headerByteSize;
    const std::size_t payloadSize = buffer.size() - header.headerByteSize;

    Result<OpOutput<TriMesh>> result;
    if (header.format == PlyFormat::Ascii) {
        result = parseAsciiPly(payload, payloadSize, header);
    } else if (header.format == PlyFormat::BinaryLittleEndian) {
        result = parseBinaryPly(payload, payloadSize, header, /*isBigEndian=*/false);
    } else if (header.format == PlyFormat::BinaryBigEndian) {
        result = parseBinaryPly(payload, payloadSize, header, /*isBigEndian=*/true);
    } else {
        return tl::make_unexpected(Error{ErrorCode::IoError, "Unsupported PLY format"});
    }

    if (result) {
        const auto endTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
        result->metrics.durationMs = elapsed.count();
        result->metrics.peakMemoryBytes = buffer.size() +
                                          result->value.vertices.size() * sizeof(Vec3) +
                                          result->value.faces.size() * sizeof(Triangle);
    }

    return result;
}

Result<OpOutput<TriMesh>> readPlyWithReport(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return tl::make_unexpected(
            Error{ErrorCode::IoError, "Failed to open PLY file: " + path.string()});
    }
    return readPlyWithReport(file);
}

Result<TriMesh> readPly(std::istream& is) {
    auto res = readPlyWithReport(is);
    if (!res) {
        return tl::make_unexpected(res.error());
    }
    return res->value;
}

Result<TriMesh> readPly(const std::filesystem::path& path) {
    auto res = readPlyWithReport(path);
    if (!res) {
        return tl::make_unexpected(res.error());
    }
    return res->value;
}

}  // namespace geomera
