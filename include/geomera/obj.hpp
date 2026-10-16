#pragma once

#include <geomera/error.hpp>
#include <geomera/mesh.hpp>

#include <filesystem>
#include <istream>

namespace geomera {

[[nodiscard]] Result<OpOutput<TriMesh>> readObjWithReport(std::istream& is);
[[nodiscard]] Result<OpOutput<TriMesh>> readObjWithReport(const std::filesystem::path& path);

[[nodiscard]] Result<TriMesh> readObj(std::istream& is);
[[nodiscard]] Result<TriMesh> readObj(const std::filesystem::path& path);

}
