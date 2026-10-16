#include <gtest/gtest.h>
#include <geomera/measurements.hpp>
#include "primitives.hpp"

TEST(PrimitivesTest, TriangleProperties) {
    const auto mesh = geomera::test::makeTriangle();
    EXPECT_EQ(mesh.vertexCount(), 3U);
    EXPECT_EQ(mesh.faceCount(), 1U);
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 0.5);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(PrimitivesTest, TetrahedronProperties) {
    const auto mesh = geomera::test::makeTetrahedron();
    EXPECT_EQ(mesh.vertexCount(), 4U);
    EXPECT_EQ(mesh.faceCount(), 4U);
    EXPECT_GT(geomera::surfaceArea(mesh), 0.0);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(PrimitivesTest, BoxProperties) {
    const auto mesh = geomera::test::makeBox(
        geomera::Vec3(-1.0, -1.0, -1.0),
        geomera::Vec3(1.0, 1.0, 1.0)
    );
    EXPECT_EQ(mesh.vertexCount(), 8U);
    EXPECT_EQ(mesh.faceCount(), 12U);
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 24.0);

    const auto box = geomera::bounds(mesh);
    EXPECT_EQ(box.min, geomera::Vec3(-1.0, -1.0, -1.0));
    EXPECT_EQ(box.max, geomera::Vec3(1.0, 1.0, 1.0));
}

TEST(PrimitivesTest, IcosphereProperties) {
    const double radius = 2.5;
    const auto sphere0 = geomera::test::makeIcosphere(0, radius);
    EXPECT_EQ(sphere0.vertexCount(), 12U);
    EXPECT_EQ(sphere0.faceCount(), 20U);

    for (const auto& v : sphere0.vertices) {
        EXPECT_NEAR(v.norm(), radius, 1e-12);
    }

    const auto sphere1 = geomera::test::makeIcosphere(1, radius);
    EXPECT_EQ(sphere1.vertexCount(), 42U);
    EXPECT_EQ(sphere1.faceCount(), 80U);

    for (const auto& v : sphere1.vertices) {
        EXPECT_NEAR(v.norm(), radius, 1e-12);
    }
}

TEST(PrimitivesTest, CylinderProperties) {
    const auto mesh = geomera::test::makeCylinder(1.0, 2.0, 16);
    EXPECT_EQ(mesh.vertexCount(), 34U);
    EXPECT_EQ(mesh.faceCount(), 64U);
    EXPECT_GT(geomera::surfaceArea(mesh), 0.0);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(PrimitivesTest, TorusProperties) {
    const auto mesh = geomera::test::makeTorus(2.0, 0.5, 16, 8);
    EXPECT_EQ(mesh.vertexCount(), 128U);
    EXPECT_EQ(mesh.faceCount(), 256U);
    EXPECT_GT(geomera::surfaceArea(mesh), 0.0);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(PrimitivesTest, Determinism) {
    const auto b1 = geomera::test::makeBox();
    const auto b2 = geomera::test::makeBox();
    ASSERT_EQ(b1.vertices.size(), b2.vertices.size());
    ASSERT_EQ(b1.faces.size(), b2.faces.size());

    for (std::size_t i = 0; i < b1.vertices.size(); ++i) {
        EXPECT_EQ(b1.vertices[i], b2.vertices[i]);
    }
    for (std::size_t i = 0; i < b1.faces.size(); ++i) {
        EXPECT_EQ(b1.faces[i], b2.faces[i]);
    }
}
