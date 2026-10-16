#include <geomera/obj.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace geomera {

namespace {

Result<int64_t> parseVertexIndex(std::string_view token) {
    if (token.empty()) {
        return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Empty face index token"});
    }
    const auto slashPos = token.find('/');
    const std::string_view vStr = (slashPos == std::string_view::npos) ? token : token.substr(0, slashPos);
    if (vStr.empty()) {
        return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Missing vertex index in face definition"});
    }
    try {
        std::size_t idx = 0;
        const std::string s(vStr);
        const int64_t val = std::stoll(s, &idx);
        if (idx != s.size()) {
            return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Malformed vertex index in face: " + s});
        }
        return val;
    } catch (...) {
        return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Invalid integer in face token: " + std::string(vStr)});
    }
}

}

Result<OpOutput<TriMesh>> readObjWithReport(std::istream& is) {
    const auto startTime = std::chrono::steady_clock::now();

    TriMesh mesh;
    std::vector<Warning> warnings;
    std::string line;
    bool hasAnyData = false;

    while (std::getline(is, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::size_t start = 0;
        while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) {
            ++start;
        }
        if (start >= line.size()) {
            continue;
        }

        hasAnyData = true;

        if (line[start] == '#') {
            continue;
        }

        std::istringstream lineStream(line.substr(start));
        std::string tag;
        if (!(lineStream >> tag)) {
            continue;
        }

        if (tag == "v") {
            std::string xs;
            std::string ys;
            std::string zs;
            if (!(lineStream >> xs >> ys >> zs)) {
                return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Malformed vertex record in OBJ: " + line});
            }
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            try {
                x = std::stod(xs);
                y = std::stod(ys);
                z = std::stod(zs);
            } catch (...) {
                return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Non-numeric coordinate in OBJ: " + line});
            }
            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
                return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "Non-finite vertex coordinates in OBJ"});
            }
            mesh.vertices.emplace_back(x, y, z);
        } else if (tag == "f") {
            std::vector<Index> faceVertices;
            std::string token;
            while (lineStream >> token) {
                auto idxRes = parseVertexIndex(token);
                if (!idxRes) {
                    return tl::make_unexpected(idxRes.error());
                }
                const int64_t rawIdx = *idxRes;
                if (rawIdx == 0) {
                    return tl::make_unexpected(Error{ErrorCode::InvalidMesh, "OBJ face indices cannot be zero"});
                }

                int64_t resolvedIdx = 0;
                if (rawIdx > 0) {
                    resolvedIdx = rawIdx - 1;
                } else {
                    resolvedIdx = static_cast<int64_t>(mesh.vertices.size()) + rawIdx;
                }

                if (resolvedIdx < 0 || static_cast<std::size_t>(resolvedIdx) >= mesh.vertices.size()) {
                    return tl::make_unexpected(Error{
                        ErrorCode::InvalidMesh,
                        "Out-of-bounds vertex index " + std::to_string(rawIdx) + " in OBJ face"
                    });
                }
                faceVertices.push_back(static_cast<Index>(resolvedIdx));
            }

            if (faceVertices.empty()) {
                continue;
            }

            if (faceVertices.size() < 3) {
                return tl::make_unexpected(Error{
                    ErrorCode::InvalidMesh,
                    "Face record with fewer than 3 vertices in OBJ"
                });
            }

            if (faceVertices.size() > 3) {
                warnings.push_back(Warning{
                    "OBJ_POLYGON_TRIANGULATED",
                    "Face with " + std::to_string(faceVertices.size()) + " vertices fan-triangulated",
                    {{"vertex_count", faceVertices.size()}}
                });
            }

            for (std::size_t k = 1; k + 1 < faceVertices.size(); ++k) {
                mesh.faces.push_back(Triangle{faceVertices[0], faceVertices[k], faceVertices[k + 1]});
            }
        }
    }

    if (!hasAnyData) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "Empty OBJ input"});
    }

    const auto endTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    OpOutput<TriMesh> result;
    result.metrics.durationMs = elapsed.count();
    result.metrics.peakMemoryBytes = mesh.vertices.size() * sizeof(Vec3) + mesh.faces.size() * sizeof(Triangle);
    result.value = std::move(mesh);
    result.warnings = std::move(warnings);
    return result;
}

Result<OpOutput<TriMesh>> readObjWithReport(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return tl::make_unexpected(Error{ErrorCode::IoError, "Failed to open OBJ file: " + path.string()});
    }
    return readObjWithReport(file);
}

Result<TriMesh> readObj(std::istream& is) {
    auto res = readObjWithReport(is);
    if (!res) {
        return tl::make_unexpected(res.error());
    }
    return res->value;
}

Result<TriMesh> readObj(const std::filesystem::path& path) {
    auto res = readObjWithReport(path);
    if (!res) {
        return tl::make_unexpected(res.error());
    }
    return res->value;
}

}
