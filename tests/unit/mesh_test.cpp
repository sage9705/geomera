#include <gtest/gtest.h>
#include <geomera/mesh.hpp>

TEST(MeshTest, EmptyMesh) {
    geomera::TriMesh mesh;
    EXPECT_TRUE(mesh.empty());
    EXPECT_EQ(mesh.vertexCount(), 0U);
    EXPECT_EQ(mesh.faceCount(), 0U);
}

TEST(MeshTest, SingleTriangle) {
    geomera::TriMesh mesh;
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(1.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(0.0, 1.0, 0.0));
    mesh.faces.push_back({0, 1, 2});

    EXPECT_FALSE(mesh.empty());
    EXPECT_EQ(mesh.vertexCount(), 3U);
    EXPECT_EQ(mesh.faceCount(), 1U);
}

TEST(MeshTest, InvalidIndexDistinctFromValid) {
    const geomera::Index validIndex = 0;
    EXPECT_NE(geomera::kInvalidIndex, validIndex);
    EXPECT_GT(geomera::kInvalidIndex, 1000000U);
}

TEST(AabbTest, EmptyAndDefault) {
    geomera::Aabb box;
    EXPECT_TRUE(box.isEmpty());
    EXPECT_FALSE(box.contains(geomera::Vec3(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(box.surfaceArea(), 0.0);
    EXPECT_DOUBLE_EQ(box.diagonalLength(), 0.0);
}

TEST(AabbTest, ExpandAndContainment) {
    geomera::Aabb box;
    box.expand(geomera::Vec3(1.0, 2.0, 3.0));
    box.expand(geomera::Vec3(4.0, 6.0, 8.0));

    EXPECT_FALSE(box.isEmpty());
    EXPECT_TRUE(box.contains(geomera::Vec3(1.0, 2.0, 3.0)));
    EXPECT_TRUE(box.contains(geomera::Vec3(4.0, 6.0, 8.0)));
    EXPECT_TRUE(box.contains(geomera::Vec3(2.5, 4.0, 5.5)));
    EXPECT_FALSE(box.contains(geomera::Vec3(0.9, 2.0, 3.0)));
    EXPECT_FALSE(box.contains(geomera::Vec3(4.1, 6.0, 8.0)));
}

TEST(AabbTest, SurfaceAreaAndDiagonal) {
    geomera::Aabb box(geomera::Vec3(0.0, 0.0, 0.0), geomera::Vec3(2.0, 3.0, 4.0));
    EXPECT_FALSE(box.isEmpty());
    EXPECT_DOUBLE_EQ(box.surfaceArea(), 2.0 * (2.0 * 3.0 + 3.0 * 4.0 + 4.0 * 2.0));
    EXPECT_DOUBLE_EQ(box.diagonalLength(), std::sqrt(4.0 + 9.0 + 16.0));
    EXPECT_EQ(box.center(), geomera::Vec3(1.0, 1.5, 2.0));
}

TEST(AabbTest, ContainsOtherAabb) {
    geomera::Aabb outer(geomera::Vec3(0.0, 0.0, 0.0), geomera::Vec3(10.0, 10.0, 10.0));
    geomera::Aabb inner(geomera::Vec3(1.0, 1.0, 1.0), geomera::Vec3(9.0, 9.0, 9.0));
    geomera::Aabb overlapping(geomera::Vec3(5.0, 5.0, 5.0), geomera::Vec3(15.0, 15.0, 15.0));

    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(outer.contains(overlapping));
    EXPECT_FALSE(inner.contains(outer));
}
