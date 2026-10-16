#include <geomera/measurements.hpp>
#include <geomera/obj.hpp>
#include <primitives.hpp>

#include <sstream>
#include <string>
#include <gtest/gtest.h>

namespace geomera {
namespace {

TEST(ObjTest, BasicSingleTriangle) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 2.0 0.0 0.0\n"
        "v 0.0 3.0 0.0\n"
        "f 1 2 3\n";

    std::istringstream stream(content);
    auto res = readObjWithReport(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    const auto& mesh = res->value;
    EXPECT_EQ(mesh.vertexCount(), 3u);
    EXPECT_EQ(mesh.faceCount(), 1u);
    EXPECT_TRUE(res->warnings.empty());

    EXPECT_DOUBLE_EQ(mesh.vertices[0].x(), 0.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[1].x(), 2.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[2].y(), 3.0);
    EXPECT_EQ(mesh.faces[0], (Triangle{0, 1, 2}));
}

TEST(ObjTest, FaceForms) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "vt 0.0 0.0\n"
        "vn 0.0 0.0 1.0\n"
        "f 1/1/1 2/1/1 3/1/1\n"
        "f 1/1 2/1 3/1\n"
        "f 1//1 2//1 3//1\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 3u);
    EXPECT_EQ(res->faces[0], (Triangle{0, 1, 2}));
    EXPECT_EQ(res->faces[1], (Triangle{0, 1, 2}));
    EXPECT_EQ(res->faces[2], (Triangle{0, 1, 2}));
}

TEST(ObjTest, NegativeIndices) {
    const std::string content =
        "v 10.0 0.0 0.0\n"
        "v 20.0 0.0 0.0\n"
        "v 30.0 0.0 0.0\n"
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f -3 -2 -1\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 6u);
    EXPECT_EQ(res->faceCount(), 1u);
    EXPECT_EQ(res->faces[0], (Triangle{3, 4, 5}));
}

TEST(ObjTest, QuadFanTriangulation) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f 1 2 3 4\n";

    std::istringstream stream(content);
    auto res = readObjWithReport(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    const auto& mesh = res->value;
    EXPECT_EQ(mesh.vertexCount(), 4u);
    EXPECT_EQ(mesh.faceCount(), 2u);
    EXPECT_EQ(mesh.faces[0], (Triangle{0, 1, 2}));
    EXPECT_EQ(mesh.faces[1], (Triangle{0, 2, 3}));

    ASSERT_EQ(res->warnings.size(), 1u);
    EXPECT_EQ(res->warnings[0].code, "OBJ_POLYGON_TRIANGULATED");
}

TEST(ObjTest, IgnoreUnsupportedRecords) {
    const std::string content =
        "# Comment line\n"
        "o TestObject\n"
        "g TestGroup\n"
        "s 1\n"
        "mtllib test.mtl\n"
        "usemtl DefaultMaterial\n"
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "vt 0.5 0.5\n"
        "vn 0 0 1\n"
        "f 1 2 3\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_TRUE(res.has_value()) << res.error().message;

    EXPECT_EQ(res->vertexCount(), 3u);
    EXPECT_EQ(res->faceCount(), 1u);
}

TEST(ObjTest, ErrorEmptyInput) {
    std::istringstream stream("");
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::IoError);
}

TEST(ObjTest, ErrorZeroIndex) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f 0 1 2\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, ErrorOutOfBoundsPositiveIndex) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f 1 2 10\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, ErrorOutOfBoundsNegativeIndex) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f -10 -2 -1\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, ErrorNonFiniteCoordinate) {
    const std::string content =
        "v nan 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f 1 2 3\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, ErrorFewerThanThreeVerticesInFace) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "f 1 2\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, ErrorMalformedFaceToken) {
    const std::string content =
        "v 0.0 0.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "f 1 not_a_number 3\n";

    std::istringstream stream(content);
    auto res = readObj(stream);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidMesh);
}

TEST(ObjTest, WriteRoundTripBox) {
    const auto box = test::makeBox();
    std::stringstream ss;
    const auto writeRes = writeObj(ss, box);
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = readObj(ss);
    ASSERT_TRUE(readRes.has_value());
    EXPECT_EQ(readRes->faces.size(), box.faces.size());
    EXPECT_EQ(readRes->vertices.size(), box.vertices.size());
    EXPECT_NEAR(surfaceArea(readRes.value()), surfaceArea(box), 1e-4);
}

TEST(ObjTest, WriteRoundTripTetrahedron) {
    const auto tetra = test::makeTetrahedron();
    std::stringstream ss;
    const auto writeRes = writeObj(ss, tetra);
    ASSERT_TRUE(writeRes.has_value());

    const auto readRes = readObj(ss);
    ASSERT_TRUE(readRes.has_value());
    EXPECT_EQ(readRes->faces.size(), tetra.faces.size());
    EXPECT_EQ(readRes->vertices.size(), tetra.vertices.size());
    EXPECT_NEAR(surfaceArea(readRes.value()), surfaceArea(tetra), 1e-4);
}

}
}
