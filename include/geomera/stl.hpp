#pragma once

#include <filesystem>
#include <istream>
#include <ostream>
#include <string>
#include <geomera/error.hpp>
#include <geomera/mesh.hpp>

namespace geomera {

enum class StlFormat {
    Ascii,
    Binary
};

[[nodiscard]] Result<TriMesh> readStl(std::istream& is);
[[nodiscard]] Result<TriMesh> readStl(const std::filesystem::path& path);

[[nodiscard]] Result<void> writeStl(
    std::ostream& os,
    const TriMesh& mesh,
    StlFormat format = StlFormat::Binary,
    const std::string& solidNameOrHeader = ""
);

[[nodiscard]] Result<void> writeStl(
    const std::filesystem::path& path,
    const TriMesh& mesh,
    StlFormat format = StlFormat::Binary,
    const std::string& solidNameOrHeader = ""
);

}
