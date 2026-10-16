#include <gtest/gtest.h>
#include <geomera/units.hpp>

TEST(UnitsTest, ToMillimetersScale) {
    EXPECT_DOUBLE_EQ(geomera::toMillimetersScale(geomera::LengthUnit::Millimeter), 1.0);
    EXPECT_DOUBLE_EQ(geomera::toMillimetersScale(geomera::LengthUnit::Centimeter), 10.0);
    EXPECT_DOUBLE_EQ(geomera::toMillimetersScale(geomera::LengthUnit::Meter), 1000.0);
    EXPECT_DOUBLE_EQ(geomera::toMillimetersScale(geomera::LengthUnit::Inch), 25.4);
    EXPECT_DOUBLE_EQ(geomera::toMillimetersScale(geomera::LengthUnit::Foot), 304.8);
}

TEST(UnitsTest, FromMillimetersScale) {
    EXPECT_DOUBLE_EQ(geomera::fromMillimetersScale(geomera::LengthUnit::Millimeter), 1.0);
    EXPECT_DOUBLE_EQ(geomera::fromMillimetersScale(geomera::LengthUnit::Centimeter), 0.1);
    EXPECT_DOUBLE_EQ(geomera::fromMillimetersScale(geomera::LengthUnit::Meter), 0.001);
    EXPECT_DOUBLE_EQ(geomera::fromMillimetersScale(geomera::LengthUnit::Inch), 1.0 / 25.4);
    EXPECT_DOUBLE_EQ(geomera::fromMillimetersScale(geomera::LengthUnit::Foot), 1.0 / 304.8);
}

TEST(UnitsTest, ParseValidUnits) {
    const auto mm = geomera::parseLengthUnit("mm");
    ASSERT_TRUE(mm.has_value());
    EXPECT_EQ(mm.value(), geomera::LengthUnit::Millimeter);

    const auto cm = geomera::parseLengthUnit("CM");
    ASSERT_TRUE(cm.has_value());
    EXPECT_EQ(cm.value(), geomera::LengthUnit::Centimeter);

    const auto m = geomera::parseLengthUnit("meter");
    ASSERT_TRUE(m.has_value());
    EXPECT_EQ(m.value(), geomera::LengthUnit::Meter);

    const auto in = geomera::parseLengthUnit("inches");
    ASSERT_TRUE(in.has_value());
    EXPECT_EQ(in.value(), geomera::LengthUnit::Inch);

    const auto ft = geomera::parseLengthUnit("feet");
    ASSERT_TRUE(ft.has_value());
    EXPECT_EQ(ft.value(), geomera::LengthUnit::Foot);
}

TEST(UnitsTest, ParseInvalidUnits) {
    const auto err = geomera::parseLengthUnit("yards");
    EXPECT_FALSE(err.has_value());
    EXPECT_EQ(err.error().code, geomera::ErrorCode::OperationFailed);
}

TEST(UnitsTest, ToString) {
    EXPECT_EQ(geomera::lengthUnitToString(geomera::LengthUnit::Millimeter), "mm");
    EXPECT_EQ(geomera::lengthUnitToString(geomera::LengthUnit::Centimeter), "cm");
    EXPECT_EQ(geomera::lengthUnitToString(geomera::LengthUnit::Meter), "m");
    EXPECT_EQ(geomera::lengthUnitToString(geomera::LengthUnit::Inch), "in");
    EXPECT_EQ(geomera::lengthUnitToString(geomera::LengthUnit::Foot), "ft");
}

TEST(UnitsTest, ScaleMesh) {
    geomera::TriMesh mesh;
    mesh.vertices.push_back({1.0, 2.0, 3.0});
    mesh.faces.push_back({0, 0, 0});

    geomera::scaleMesh(mesh, 10.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[0].x(), 10.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[0].y(), 20.0);
    EXPECT_DOUBLE_EQ(mesh.vertices[0].z(), 30.0);
}
