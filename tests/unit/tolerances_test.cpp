#include <limits>
#include <gtest/gtest.h>
#include <geomera/tolerances.hpp>

TEST(TolerancesTest, DefaultTolerancesAreValid) {
    const geomera::Tolerances tol;
    EXPECT_TRUE(tol.isValid());
    EXPECT_GT(tol.pointCoincidenceMm, 0.0);
    EXPECT_GT(tol.degenerateAreaMm2, 0.0);
    EXPECT_GT(tol.coplanarAngleRad, 0.0);
    EXPECT_GT(tol.planeDistanceMm, 0.0);
    EXPECT_GT(tol.icpConvergenceRelative, 0.0);
    EXPECT_GT(tol.qemInversionThreshold, 0.0);
}

TEST(TolerancesTest, CustomTolerancesConfiguration) {
    geomera::Tolerances tol;
    tol.pointCoincidenceMm = 1e-4;
    tol.degenerateAreaMm2 = 1e-8;

    EXPECT_TRUE(tol.isValid());
    EXPECT_DOUBLE_EQ(tol.pointCoincidenceMm, 1e-4);
    EXPECT_DOUBLE_EQ(tol.degenerateAreaMm2, 1e-8);
    EXPECT_NE(tol, geomera::kDefaultTolerances);
}

TEST(TolerancesTest, InvalidTolerancesDetected) {
    geomera::Tolerances zeroTol;
    zeroTol.pointCoincidenceMm = 0.0;
    EXPECT_FALSE(zeroTol.isValid());

    geomera::Tolerances negativeTol;
    negativeTol.degenerateAreaMm2 = -1e-6;
    EXPECT_FALSE(negativeTol.isValid());

    geomera::Tolerances nanTol;
    nanTol.coplanarAngleRad = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(nanTol.isValid());

    geomera::Tolerances infTol;
    infTol.planeDistanceMm = std::numeric_limits<double>::infinity();
    EXPECT_FALSE(infTol.isValid());
}
