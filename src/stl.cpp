#include <geomera/stl.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace geomera {

namespace {

struct Float3Bits {
    uint32_t x{0};
    uint32_t y{0};
    uint32_t z{0};

    bool operator==(const Float3Bits& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Float3BitsHash {
    std::size_t operator()(const Float3Bits& k) const noexcept {
        std::size_t h = 2166136261u;
        auto combine = [&h](uint32_t v) {
            h ^= static_cast<std::size_t>(v);
            h *= 16777619u;
        };
        combine(k.x);
        combine(k.y);
        combine(k.z);
        return h;
    }
};

std::string toLower(std::string_view s) {
    std::string res;
    res.reserve(s.size());
    for (char c : s) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}

bool startsWithSolid(const std::vector<uint8_t>& buf) {
    std::size_t i = 0;
    while (i < buf.size() && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\r' || buf[i] == '\n')) {
        ++i;
    }
    if (i + 5 > buf.size()) {
        return false;
    }
    const char expected[] = "solid";
    for (std::size_t k = 0; k < 5; ++k) {
        if (std::tolower(static_cast<unsigned char>(buf[i + k])) != expected[k]) {
            return false;
        }
    }
    return true;
}

Result<TriMesh> parseBinaryStl(const std::vector<uint8_t>& buf) {
    if (buf.size() < 84) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Binary STL file truncated: header is smaller than 84 bytes"
        });
    }

    uint32_t triangleCount = 0;
    std::memcpy(&triangleCount, &buf[80], sizeof(uint32_t));

    const uint64_t expectedSize = 84ULL + 50ULL * static_cast<uint64_t>(triangleCount);
    if (buf.size() < expectedSize) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Binary STL file truncated: expected " + std::to_string(expectedSize) +
                " bytes for " + std::to_string(triangleCount) + " triangles, but found " +
                std::to_string(buf.size()) + " bytes"
        });
    }

    TriMesh mesh;
    mesh.faces.reserve(triangleCount);

    std::unordered_map<Float3Bits, Index, Float3BitsHash> vertexMap;

    std::size_t offset = 84;
    for (uint32_t i = 0; i < triangleCount; ++i) {
        offset += 12;

        Triangle faceIndices{};
        for (std::size_t v = 0; v < 3; ++v) {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            std::memcpy(&x, &buf[offset], sizeof(float));
            std::memcpy(&y, &buf[offset + 4], sizeof(float));
            std::memcpy(&z, &buf[offset + 8], sizeof(float));
            offset += 12;

            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
                return tl::make_unexpected(Error{
                    ErrorCode::InvalidMesh,
                    "Non-finite coordinate encountered in STL mesh"
                });
            }

            if (x == 0.0f) x = 0.0f;
            if (y == 0.0f) y = 0.0f;
            if (z == 0.0f) z = 0.0f;

            Float3Bits key;
            std::memcpy(&key.x, &x, sizeof(float));
            std::memcpy(&key.y, &y, sizeof(float));
            std::memcpy(&key.z, &z, sizeof(float));

            auto it = vertexMap.find(key);
            if (it != vertexMap.end()) {
                faceIndices[v] = it->second;
            } else {
                const auto newIdx = static_cast<Index>(mesh.vertices.size());
                mesh.vertices.emplace_back(
                    static_cast<double>(x),
                    static_cast<double>(y),
                    static_cast<double>(z)
                );
                vertexMap.emplace(key, newIdx);
                faceIndices[v] = newIdx;
            }
        }

        offset += 2;
        mesh.faces.push_back(faceIndices);
    }

    return mesh;
}

Result<TriMesh> parseAsciiStl(const std::vector<uint8_t>& buf) {
    std::string text(reinterpret_cast<const char*>(buf.data()), buf.size());
    std::istringstream stream(text);

    std::string token;
    if (!(stream >> token)) {
        return tl::make_unexpected(Error{
            ErrorCode::InvalidMesh,
            "Empty ASCII STL stream"
        });
    }

    if (toLower(token) != "solid") {
        return tl::make_unexpected(Error{
            ErrorCode::InvalidMesh,
            "Expected 'solid' at start of ASCII STL"
        });
    }

    std::string line;
    std::getline(stream, line);

    TriMesh mesh;
    std::unordered_map<Float3Bits, Index, Float3BitsHash> vertexMap;

    while (stream >> token) {
        const std::string lowerToken = toLower(token);
        if (lowerToken == "endsolid") {
            break;
        }

        if (lowerToken != "facet") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'facet' or 'endsolid', got: " + token
            });
        }

        if (!(stream >> token) || toLower(token) != "normal") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'normal' after 'facet'"
            });
        }

        float nx = 0.0f;
        float ny = 0.0f;
        float nz = 0.0f;
        if (!(stream >> nx >> ny >> nz)) {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Failed to read normal components"
            });
        }

        if (!(stream >> token) || toLower(token) != "outer") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'outer' keyword"
            });
        }

        if (!(stream >> token) || toLower(token) != "loop") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'loop' keyword"
            });
        }

        Triangle faceIndices{};
        for (std::size_t v = 0; v < 3; ++v) {
            if (!(stream >> token) || toLower(token) != "vertex") {
                return tl::make_unexpected(Error{
                    ErrorCode::InvalidMesh,
                    "Expected 'vertex' keyword"
                });
            }

            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            if (!(stream >> x >> y >> z)) {
                return tl::make_unexpected(Error{
                    ErrorCode::InvalidMesh,
                    "Failed to read vertex coordinates"
                });
            }

            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
                return tl::make_unexpected(Error{
                    ErrorCode::InvalidMesh,
                    "Non-finite coordinate encountered in ASCII STL mesh"
                });
            }

            if (x == 0.0f) x = 0.0f;
            if (y == 0.0f) y = 0.0f;
            if (z == 0.0f) z = 0.0f;

            Float3Bits key;
            std::memcpy(&key.x, &x, sizeof(float));
            std::memcpy(&key.y, &y, sizeof(float));
            std::memcpy(&key.z, &z, sizeof(float));

            auto it = vertexMap.find(key);
            if (it != vertexMap.end()) {
                faceIndices[v] = it->second;
            } else {
                const auto newIdx = static_cast<Index>(mesh.vertices.size());
                mesh.vertices.emplace_back(
                    static_cast<double>(x),
                    static_cast<double>(y),
                    static_cast<double>(z)
                );
                vertexMap.emplace(key, newIdx);
                faceIndices[v] = newIdx;
            }
        }

        if (!(stream >> token) || toLower(token) != "endloop") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'endloop' keyword"
            });
        }

        if (!(stream >> token) || toLower(token) != "endfacet") {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidMesh,
                "Expected 'endfacet' keyword"
            });
        }

        mesh.faces.push_back(faceIndices);
    }

    return mesh;
}

}

Result<TriMesh> readStl(std::istream& is) {
    std::vector<uint8_t> buffer(
        (std::istreambuf_iterator<char>(is)),
        std::istreambuf_iterator<char>()
    );

    if (buffer.empty()) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Input stream is empty"
        });
    }

    if (buffer.size() >= 84) {
        uint32_t triangleCount = 0;
        std::memcpy(&triangleCount, &buffer[80], sizeof(uint32_t));
        const uint64_t expectedBinarySize = 84ULL + 50ULL * static_cast<uint64_t>(triangleCount);

        if (buffer.size() == expectedBinarySize) {
            return parseBinaryStl(buffer);
        }

        if (startsWithSolid(buffer)) {
            return parseAsciiStl(buffer);
        }

        if (buffer.size() < expectedBinarySize) {
            return tl::make_unexpected(Error{
                ErrorCode::IoError,
                "Binary STL file truncated: expected " + std::to_string(expectedBinarySize) +
                    " bytes for " + std::to_string(triangleCount) + " triangles, but found " +
                    std::to_string(buffer.size()) + " bytes"
            });
        }

        return tl::make_unexpected(Error{
            ErrorCode::InvalidMesh,
            "Malformed STL file: does not match binary triangle count and is not valid ASCII STL"
        });
    }

    if (startsWithSolid(buffer)) {
        return parseAsciiStl(buffer);
    }

    return tl::make_unexpected(Error{
        ErrorCode::IoError,
        "Truncated STL header: input is smaller than 84 bytes and not valid ASCII STL"
    });
}

Result<TriMesh> readStl(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Failed to open STL file: " + path.string()
        });
    }
    return readStl(file);
}

Result<void> writeStl(
    std::ostream& os,
    const TriMesh& mesh,
    StlFormat format,
    const std::string& solidNameOrHeader
) {
    if (format == StlFormat::Binary) {
        std::array<char, 80> header{};
        header.fill(0);
        const std::string headerText = solidNameOrHeader.empty()
            ? "Geomera binary STL"
            : solidNameOrHeader;
        const std::size_t copyLen = std::min(headerText.size(), std::size_t{80});
        std::memcpy(header.data(), headerText.data(), copyLen);
        os.write(header.data(), 80);

        const auto count = static_cast<uint32_t>(mesh.faces.size());
        os.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

        for (const auto& face : mesh.faces) {
            const Vec3& v0 = mesh.vertices[face[0]];
            const Vec3& v1 = mesh.vertices[face[1]];
            const Vec3& v2 = mesh.vertices[face[2]];

            Vec3 normal = (v1 - v0).cross(v2 - v0);
            const double len = normal.norm();
            if (len > 0.0) {
                normal /= len;
            } else {
                normal = Vec3::Zero();
            }

            const float fn[3] = {
                static_cast<float>(normal.x()),
                static_cast<float>(normal.y()),
                static_cast<float>(normal.z())
            };
            os.write(reinterpret_cast<const char*>(fn), 12);

            const float fv0[3] = {
                static_cast<float>(v0.x()),
                static_cast<float>(v0.y()),
                static_cast<float>(v0.z())
            };
            os.write(reinterpret_cast<const char*>(fv0), 12);

            const float fv1[3] = {
                static_cast<float>(v1.x()),
                static_cast<float>(v1.y()),
                static_cast<float>(v1.z())
            };
            os.write(reinterpret_cast<const char*>(fv1), 12);

            const float fv2[3] = {
                static_cast<float>(v2.x()),
                static_cast<float>(v2.y()),
                static_cast<float>(v2.z())
            };
            os.write(reinterpret_cast<const char*>(fv2), 12);

            const uint16_t attr = 0;
            os.write(reinterpret_cast<const char*>(&attr), sizeof(uint16_t));
        }

        if (!os) {
            return tl::make_unexpected(Error{
                ErrorCode::IoError,
                "Failed to write binary STL stream"
            });
        }
        return {};
    }

    const std::string name = solidNameOrHeader.empty() ? "geomera" : solidNameOrHeader;
    os << "solid " << name << "\n";

    for (const auto& face : mesh.faces) {
        const Vec3& v0 = mesh.vertices[face[0]];
        const Vec3& v1 = mesh.vertices[face[1]];
        const Vec3& v2 = mesh.vertices[face[2]];

        Vec3 normal = (v1 - v0).cross(v2 - v0);
        const double len = normal.norm();
        if (len > 0.0) {
            normal /= len;
        } else {
            normal = Vec3::Zero();
        }

        os << "  facet normal " << static_cast<float>(normal.x()) << " "
           << static_cast<float>(normal.y()) << " " << static_cast<float>(normal.z()) << "\n";
        os << "    outer loop\n";
        os << "      vertex " << static_cast<float>(v0.x()) << " "
           << static_cast<float>(v0.y()) << " " << static_cast<float>(v0.z()) << "\n";
        os << "      vertex " << static_cast<float>(v1.x()) << " "
           << static_cast<float>(v1.y()) << " " << static_cast<float>(v1.z()) << "\n";
        os << "      vertex " << static_cast<float>(v2.x()) << " "
           << static_cast<float>(v2.y()) << " " << static_cast<float>(v2.z()) << "\n";
        os << "    endloop\n";
        os << "  endfacet\n";
    }

    os << "endsolid " << name << "\n";

    if (!os) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Failed to write ASCII STL stream"
        });
    }
    return {};
}

Result<void> writeStl(
    const std::filesystem::path& path,
    const TriMesh& mesh,
    StlFormat format,
    const std::string& solidNameOrHeader
) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return tl::make_unexpected(Error{
            ErrorCode::IoError,
            "Failed to open destination file for STL output: " + path.string()
        });
    }
    return writeStl(file, mesh, format, solidNameOrHeader);
}

}
