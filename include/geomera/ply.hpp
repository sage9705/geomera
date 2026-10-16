#pragma once

#include <geomera/error.hpp>
#include <geomera/mesh.hpp>

#include <filesystem>
#include <istream>

namespace geomera {

enum class PlyFormat { Ascii, BinaryLittleEndian, BinaryBigEndian };

[[nodiscard]] Result<OpOutput<TriMesh>> readPlyWithReport(std::istream& is);
[[nodiscard]] Result<OpOutput<TriMesh>> readPlyWithReport(const std::filesystem::path& path);

[[nodiscard]] Result<TriMesh> readPly(std::istream& is);
[[nodiscard]] Result<TriMesh> readPly(const std::filesystem::path& path);

}  // namespace geomera
