#include <gtest/gtest.h>
#include <geomera/error.hpp>
#include <geomera/measurements.hpp>
#include <geomera/stl.hpp>
#include <primitives.hpp>

#include <cstring>
#include <filesystem>
#include <limits>
#include <sstream>
#include <vector>

TEST(StlTest, OrdinaryBinaryRoundTrip) {
    const auto box = geomera::test::makeBox();
    ASSERT_EQ(box.faces.size(), 12U);
    ASSERT_EQ(box.vertices.size(), 8U);

    std::stringstream ss;
    const auto writeRes = geomera::writeStl(ss, box, geomera::StlFormat::Binary);
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = geomera::readStl(ss);
    ASSERT_TRUE(readRes.has_value());

    const auto& loaded = readRes.value();
    EXPECT_EQ(loaded.faces.size(), 12U);
    EXPECT_EQ(loaded.vertices.size(), 8U);

    const auto boxBounds = geomera::bounds(box);
    const auto loadedBounds = geomera::bounds(loaded);
    EXPECT_TRUE(boxBounds.min.isApprox(loadedBounds.min, 1e-5));
    EXPECT_TRUE(boxBounds.max.isApprox(loadedBounds.max, 1e-5));
    EXPECT_NEAR(geomera::surfaceArea(box), geomera::surfaceArea(loaded), 1e-4);
}

TEST(StlTest, AsciiRoundTrip) {
    const auto tetra = geomera::test::makeTetrahedron();
    ASSERT_EQ(tetra.faces.size(), 4U);
    ASSERT_EQ(tetra.vertices.size(), 4U);

    std::stringstream ss;
    const auto writeRes = geomera::writeStl(ss, tetra, geomera::StlFormat::Ascii, "tetrahedron");
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = geomera::readStl(ss);
    ASSERT_TRUE(readRes.has_value());

    const auto& loaded = readRes.value();
    EXPECT_EQ(loaded.faces.size(), 4U);
    EXPECT_EQ(loaded.vertices.size(), 4U);
    EXPECT_NEAR(geomera::surfaceArea(tetra), geomera::surfaceArea(loaded), 1e-4);
}

TEST(StlTest, BinaryHeaderBeginningWithSolid) {
    const auto box = geomera::test::makeBox();
    std::stringstream ss;
    const auto writeRes = geomera::writeStl(
        ss,
        box,
        geomera::StlFormat::Binary,
        "solid binary header test"
    );
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = geomera::readStl(ss);
    ASSERT_TRUE(readRes.has_value());

    const auto& loaded = readRes.value();
    EXPECT_EQ(loaded.faces.size(), 12U);
    EXPECT_EQ(loaded.vertices.size(), 8U);
}

TEST(StlTest, EmptyInputReturnsError) {
    std::stringstream ss("");
    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
    EXPECT_EQ(readRes.error().code, geomera::ErrorCode::IoError);
}

TEST(StlTest, GarbageBytesReturnError) {
    std::stringstream ss("Random garbage text that is definitely not STL in any form");
    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
}

TEST(StlTest, TruncatedHeaderReturnsError) {
    std::vector<uint8_t> truncated(50, 0);
    std::string str(reinterpret_cast<char*>(truncated.data()), truncated.size());
    std::stringstream ss(str);

    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
    EXPECT_EQ(readRes.error().code, geomera::ErrorCode::IoError);
}

TEST(StlTest, TruncatedTriangleReturnsError) {
    std::vector<uint8_t> bytes(84 + 50, 0);
    const uint32_t count = 2;
    std::memcpy(&bytes[80], &count, sizeof(uint32_t));

    std::string str(reinterpret_cast<char*>(bytes.data()), bytes.size());
    std::stringstream ss(str);

    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
    EXPECT_EQ(readRes.error().code, geomera::ErrorCode::IoError);
}

TEST(StlTest, NonFiniteCoordinateBinaryReturnsError) {
    std::vector<uint8_t> bytes(84 + 50, 0);
    const uint32_t count = 1;
    std::memcpy(&bytes[80], &count, sizeof(uint32_t));

    const float nanVal = std::numeric_limits<float>::quiet_NaN();
    std::memcpy(&bytes[84 + 12], &nanVal, sizeof(float));

    std::string str(reinterpret_cast<char*>(bytes.data()), bytes.size());
    std::stringstream ss(str);

    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
    EXPECT_EQ(readRes.error().code, geomera::ErrorCode::InvalidMesh);
}

TEST(StlTest, NonFiniteCoordinateAsciiReturnsError) {
    const std::string ascii =
        "solid test\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "      vertex nan 0 0\n"
        "      vertex 1 0 0\n"
        "      vertex 0 1 0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";

    std::stringstream ss(ascii);
    const auto readRes = geomera::readStl(ss);
    ASSERT_FALSE(readRes.has_value());
    EXPECT_EQ(readRes.error().code, geomera::ErrorCode::InvalidMesh);
}

TEST(StlTest, NegativeZeroNormalizedDuringExactWelding) {
    const std::string ascii =
        "solid test\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "      vertex 0 0 0\n"
        "      vertex 1 0 0\n"
        "      vertex 0 1 0\n"
        "    endloop\n"
        "  endfacet\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "      vertex -0 0 0\n"
        "      vertex 1 0 0\n"
        "      vertex 1 1 0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";

    std::stringstream ss(ascii);
    const auto readRes = geomera::readStl(ss);
    ASSERT_TRUE(readRes.has_value());

    const auto& mesh = readRes.value();
    EXPECT_EQ(mesh.faces.size(), 2U);
    EXPECT_EQ(mesh.vertices.size(), 4U);
}

TEST(StlTest, FilePathIoRoundTrip) {
    const auto box = geomera::test::makeBox();
    const auto tmpPath = std::filesystem::temp_directory_path() / "geomera_test_box.stl";

    const auto writeRes = geomera::writeStl(tmpPath, box, geomera::StlFormat::Binary);
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = geomera::readStl(tmpPath);
    ASSERT_TRUE(readRes.has_value());

    std::filesystem::remove(tmpPath);

    const auto& loaded = readRes.value();
    EXPECT_EQ(loaded.faces.size(), 12U);
    EXPECT_EQ(loaded.vertices.size(), 8U);
}
