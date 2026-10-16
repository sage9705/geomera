#pragma once

#include <geomera/error.hpp>
#include <geomera/mesh.hpp>

#include <filesystem>
#include <istream>
#include <ostream>

namespace geomera {

enum class PlyFormat { Ascii, BinaryLittleEndian, BinaryBigEndian };

struct PlyWriteOptions {
    PlyFormat format{PlyFormat::BinaryLittleEndian};
    bool useDoublePrecision{false};
};

[[nodiscard]] Result<OpOutput<TriMesh>> readPlyWithReport(std::istream& is);
[[nodiscard]] Result<OpOutput<TriMesh>> readPlyWithReport(const std::filesystem::path& path);

[[nodiscard]] Result<TriMesh> readPly(std::istream& is);
[[nodiscard]] Result<TriMesh> readPly(const std::filesystem::path& path);

[[nodiscard]] Result<void> writePly(
    std::ostream& os,
    const TriMesh& mesh,
    const PlyWriteOptions& options = {}
);

[[nodiscard]] Result<void> writePly(
    const std::filesystem::path& path,
    const TriMesh& mesh,
    const PlyWriteOptions& options = {}
);

}  // namespace geomera
