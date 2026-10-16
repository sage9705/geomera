#include <limits>
#include <gtest/gtest.h>
#include <geomera/measurements.hpp>

TEST(MeasurementsTest, EmptyMesh) {
    geomera::TriMesh mesh;
    EXPECT_TRUE(geomera::bounds(mesh).isEmpty());
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 0.0);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(MeasurementsTest, KnownTriangleAreaAndNormal) {
    geomera::TriMesh mesh;
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(2.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(0.0, 3.0, 0.0));
    mesh.faces.push_back({0, 1, 2});

    const geomera::Vec3 normal = geomera::faceNormalUnnormalized(mesh, 0);
    EXPECT_EQ(normal, geomera::Vec3(0.0, 0.0, 6.0));
    EXPECT_DOUBLE_EQ(geomera::faceArea(mesh, 0), 3.0);
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 3.0);

    const geomera::Aabb box = geomera::bounds(mesh);
    EXPECT_EQ(box.min, geomera::Vec3(0.0, 0.0, 0.0));
    EXPECT_EQ(box.max, geomera::Vec3(2.0, 3.0, 0.0));
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}

TEST(MeasurementsTest, TranslatedTriangleInvariance) {
    const geomera::Vec3 offset(10.0, -5.0, 7.0);

    geomera::TriMesh mesh;
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 0.0) + offset);
    mesh.vertices.push_back(geomera::Vec3(2.0, 0.0, 0.0) + offset);
    mesh.vertices.push_back(geomera::Vec3(0.0, 3.0, 0.0) + offset);
    mesh.faces.push_back({0, 1, 2});

    const geomera::Vec3 normal = geomera::faceNormalUnnormalized(mesh, 0);
    EXPECT_EQ(normal, geomera::Vec3(0.0, 0.0, 6.0));
    EXPECT_DOUBLE_EQ(geomera::faceArea(mesh, 0), 3.0);
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 3.0);

    const geomera::Aabb box = geomera::bounds(mesh);
    EXPECT_EQ(box.min, geomera::Vec3(10.0, -5.0, 7.0));
    EXPECT_EQ(box.max, geomera::Vec3(12.0, -2.0, 7.0));
}

TEST(MeasurementsTest, DegenerateTriangle) {
    geomera::TriMesh mesh;
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(1.0, 1.0, 1.0));
    mesh.vertices.push_back(geomera::Vec3(2.0, 2.0, 2.0));
    mesh.faces.push_back({0, 1, 2});

    const geomera::Vec3 normal = geomera::faceNormalUnnormalized(mesh, 0);
    EXPECT_EQ(normal, geomera::Vec3(0.0, 0.0, 0.0));
    EXPECT_DOUBLE_EQ(geomera::faceArea(mesh, 0), 0.0);
    EXPECT_DOUBLE_EQ(geomera::surfaceArea(mesh), 0.0);
}

TEST(MeasurementsTest, NonFiniteCoordinates) {
    geomera::TriMesh mesh;
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 0.0));
    mesh.vertices.push_back(geomera::Vec3(1.0, std::numeric_limits<double>::quiet_NaN(), 0.0));
    mesh.vertices.push_back(geomera::Vec3(0.0, 0.0, 1.0));
    mesh.faces.push_back({0, 1, 2});

    EXPECT_FALSE(geomera::hasFiniteCoordinates(mesh));

    mesh.vertices[1] = geomera::Vec3(1.0, std::numeric_limits<double>::infinity(), 0.0);
    EXPECT_FALSE(geomera::hasFiniteCoordinates(mesh));

    mesh.vertices[1] = geomera::Vec3(1.0, 2.0, 0.0);
    EXPECT_TRUE(geomera::hasFiniteCoordinates(mesh));
}
