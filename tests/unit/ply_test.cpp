#include <geomera/ply.hpp>

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace geomera {
namespace {

uint32_t toBigEndian32(uint32_t val) {
    return ((val & 0x000000FFu) << 24) | ((val & 0x0000FF00u) << 8) | ((val & 0x00FF0000u) >> 8) |
           ((val & 0xFF000000u) >> 24);
}

float floatToBigEndian(float f) {
    uint32_t u = 0;
    std::memcpy(&u, &f, sizeof(float));
    u = toBigEndian32(u);
    float res = 0.0f;
    std::memcpy(&res, &u, sizeof(float));
    return res;
}

TEST(PlyTest, AsciiSingleTriangle) {
    const std::string plyContent =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n"
        "0.0 0.0 0.0\n"
        "2.0 0.0 0.0\n"
        "0.0 3.0 0.0\n"
        "3 0 1 2\n";

    std::istringstream stream(plyContent);
    auto res = readPlyWithReport(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    const auto& mesh = res->value;
    EXPECT_EQ(mesh.vertexCount(), 3u);
    EXPECT_EQ(mesh.faceCount(), 1u);
    EXPECT_TRUE(res->warnings.empty());

    EXPECT_DOUBLE_EQ(mesh.vertices[0].x(), 0.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[1].x(), 2.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[2].y(), 3.0);
}

TEST(PlyTest, AsciiQuadFanTriangulation) {
    const std::string plyContent =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 4\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n"
        "0.0 0.0 0.0\n"
        "1.0 0.0 0.0\n"
        "1.0 1.0 0.0\n"
        "0.0 1.0 0.0\n"
        "4 0 1 2 3\n";

    std::istringstream stream(plyContent);
    auto res = readPlyWithReport(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    const auto& mesh = res->value;
    EXPECT_EQ(mesh.vertexCount(), 4u);
    EXPECT_EQ(mesh.faceCount(), 2u);

    // Quad (0, 1, 2, 3) triangulated to (0, 1, 2) and (0, 2, 3)
    EXPECT_EQ(mesh.faces[0], (Triangle{0, 1, 2}));
    EXPECT_EQ(mesh.faces[1], (Triangle{0, 2, 3}));

    ASSERT_EQ(res->warnings.size(), 1u);
    EXPECT_EQ(res->warnings[0].code, "PLY_POLYGON_TRIANGULATED");
}

TEST(PlyTest, AsciiExtraProperties) {
    const std::string plyContent =
        "ply\n"
        "format ascii 1.0\n"
        "comment test file with comments\n"
        "element vertex 3\n"
        "property float nx\n"
        "property float x\n"
        "property float y\n"
        "property float ny\n"
        "property float z\n"
        "property uchar red\n"
        "property uchar green\n"
        "property uchar blue\n"
        "element face 1\n"
        "property list uint int vertex_index\n"
        "end_header\n"
        "0.0 0.0 0.0 0.0 0.0 255 0 0\n"
        "0.0 2.0 0.0 0.0 0.0 0 255 0\n"
        "0.0 0.0 3.0 0.0 0.0 0 0 255\n"
        "3 0 1 2\n";

    std::istringstream stream(plyContent);
    auto res = readPly(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 1u);
    EXPECT_DOUBLE_EQ(res->vertices[1].x(), 2.0);
    EXPECT_DOUBLE_EQ(res->vertices[2].y(), 3.0);
}

TEST(PlyTest, BinaryLittleEndianSingleTriangle) {
    std::string header =
        "ply\n"
        "format binary_little_endian 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n";

    std::vector<uint8_t> data(header.begin(), header.end());

    float coords[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    const uint8_t* coordBytes = reinterpret_cast<const uint8_t*>(coords);
    data.insert(data.end(), coordBytes, coordBytes + sizeof(coords));

    // Face: count = 3 (uchar), indices = 0, 1, 2 (int32_t)
    uint8_t count = 3;
    int32_t indices[] = {0, 1, 2};
    data.push_back(count);
    const uint8_t* indexBytes = reinterpret_cast<const uint8_t*>(indices);
    data.insert(data.end(), indexBytes, indexBytes + sizeof(indices));

    std::string dataStr(data.begin(), data.end());
    std::istringstream stream(dataStr);
    auto res = readPly(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 1u);
    EXPECT_DOUBLE_EQ(res->vertices[1].x(), 1.0);
    EXPECT_DOUBLE_EQ(res->vertices[2].y(), 1.0);
}

TEST(PlyTest, BinaryBigEndianSingleTriangle) {
    std::string header =
        "ply\n"
        "format binary_big_endian 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n";

    std::vector<uint8_t> data(header.begin(), header.end());

    float coordsBE[] = {floatToBigEndian(0.0f),
                        floatToBigEndian(0.0f),
                        floatToBigEndian(0.0f),
                        floatToBigEndian(5.0f),
                        floatToBigEndian(0.0f),
                        floatToBigEndian(0.0f),
                        floatToBigEndian(0.0f),
                        floatToBigEndian(7.0f),
                        floatToBigEndian(0.0f)};
    const uint8_t* coordBytes = reinterpret_cast<const uint8_t*>(coordsBE);
    data.insert(data.end(), coordBytes, coordBytes + sizeof(coordsBE));

    uint8_t count = 3;
    uint32_t indicesBE[] = {toBigEndian32(0), toBigEndian32(1), toBigEndian32(2)};
    data.push_back(count);
    const uint8_t* indexBytes = reinterpret_cast<const uint8_t*>(indicesBE);
    data.insert(data.end(), indexBytes, indexBytes + sizeof(indicesBE));

    std::string dataStr(data.begin(), data.end());
    std::istringstream stream(dataStr);
    auto res = readPly(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 1u);
    EXPECT_DOUBLE_EQ(res->vertices[1].x(), 5.0);
    EXPECT_DOUBLE_EQ(res->vertices[2].y(), 7.0);
}

TEST(PlyTest, BinaryQuadFanTriangulation) {
    std::string header =
        "ply\n"
        "format binary_little_endian 1.0\n"
        "element vertex 4\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n";

    std::vector<uint8_t> data(header.begin(), header.end());

    float coords[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    const uint8_t* coordBytes = reinterpret_cast<const uint8_t*>(coords);
    data.insert(data.end(), coordBytes, coordBytes + sizeof(coords));

    uint8_t count = 4;
    int32_t indices[] = {0, 1, 2, 3};
    data.push_back(count);
    const uint8_t* indexBytes = reinterpret_cast<const uint8_t*>(indices);
    data.insert(data.end(), indexBytes, indexBytes + sizeof(indices));

    std::string dataStr(data.begin(), data.end());
    std::istringstream stream(dataStr);
    auto res = readPlyWithReport(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->value.vertexCount(), 4u);
    EXPECT_EQ(res->value.faceCount(), 2u);
    EXPECT_EQ(res->value.faces[0], (Triangle{0, 1, 2}));
    EXPECT_EQ(res->value.faces[1], (Triangle{0, 2, 3}));
    ASSERT_EQ(res->warnings.size(), 1u);
    EXPECT_EQ(res->warnings[0].code, "PLY_POLYGON_TRIANGULATED");
}

TEST(PlyTest, BinaryExtraVertexPropertiesSkipped) {
    std::string header =
        "ply\n"
        "format binary_little_endian 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "property uchar red\n"
        "property uchar green\n"
        "property uchar blue\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n";

    std::vector<uint8_t> data(header.begin(), header.end());

    auto appendVertex = [&](float x, float y, float z, uint8_t r, uint8_t g, uint8_t b) {
        float coords[3] = {x, y, z};
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(coords);
        data.insert(data.end(), ptr, ptr + sizeof(coords));
        data.push_back(r);
        data.push_back(g);
        data.push_back(b);
    };
    appendVertex(0.0f, 0.0f, 0.0f, 255, 0, 0);
    appendVertex(1.0f, 0.0f, 0.0f, 0, 255, 0);
    appendVertex(0.0f, 1.0f, 0.0f, 0, 0, 255);

    uint8_t count = 3;
    int32_t indices[] = {0, 1, 2};
    data.push_back(count);
    const uint8_t* indexBytes = reinterpret_cast<const uint8_t*>(indices);
    data.insert(data.end(), indexBytes, indexBytes + sizeof(indices));

    std::string dataStr(data.begin(), data.end());
    std::istringstream stream(dataStr);
    auto res = readPly(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 1u);
    EXPECT_DOUBLE_EQ(res->vertices[1].x(), 1.0);
}

TEST(PlyTest, ErrorNotPly) {
    std::istringstream stream("not a ply file at all");
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::IoError);
}

TEST(PlyTest, ErrorMissingEndHeader) {
    std::string content =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 3\n"
        "property float x\n";
    std::istringstream stream(content);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::IoError);
}

TEST(PlyTest, ErrorMissingCoordinateProperty) {
    std::string content =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "element face 0\n"
        "end_header\n";
    std::istringstream stream(content);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(PlyTest, ErrorOutOfBoundsIndex) {
    std::string content =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n"
        "0.0 0.0 0.0\n"
        "1.0 0.0 0.0\n"
        "0.0 1.0 0.0\n"
        "3 0 1 5\n";  // index 5 is out of bounds
    std::istringstream stream(content);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(PlyTest, ErrorNonFiniteCoordinate) {
    std::string content =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 1\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 0\n"
        "end_header\n"
        "nan 0.0 0.0\n";
    std::istringstream stream(content);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(PlyTest, ErrorTruncatedBinaryPayload) {
    std::string header =
        "ply\n"
        "format binary_little_endian 1.0\n"
        "element vertex 10\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "end_header\n";
    std::vector<uint8_t> data(header.begin(), header.end());
    // Only 1 vertex provided instead of 10
    float coords[] = {0.0f, 0.0f, 0.0f};
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(coords);
    data.insert(data.end(), ptr, ptr + sizeof(coords));

    std::string dataStr(data.begin(), data.end());
    std::istringstream stream(dataStr);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::IoError);
}

TEST(PlyTest, ErrorFaceLessThanThreeVertices) {
    std::string content =
        "ply\n"
        "format ascii 1.0\n"
        "element vertex 3\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "element face 1\n"
        "property list uchar int vertex_indices\n"
        "end_header\n"
        "0.0 0.0 0.0\n"
        "1.0 0.0 0.0\n"
        "0.0 1.0 0.0\n"
        "2 0 1\n";
    std::istringstream stream(content);
    auto res = readPly(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

}  // namespace
}  // namespace geomera
