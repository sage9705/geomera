#pragma once

#include <cmath>

namespace geomera {

struct Tolerances {
    double pointCoincidenceMm{1e-6};
    double degenerateAreaMm2{1e-12};
    double coplanarAngleRad{1e-4};
    double planeDistanceMm{1e-7};
    double icpConvergenceRelative{1e-5};
    double qemInversionThreshold{1e-8};

    [[nodiscard]] bool isValid() const noexcept {
        return pointCoincidenceMm > 0.0 && std::isfinite(pointCoincidenceMm) &&
               degenerateAreaMm2 > 0.0 && std::isfinite(degenerateAreaMm2) &&
               coplanarAngleRad > 0.0 && std::isfinite(coplanarAngleRad) &&
               planeDistanceMm > 0.0 && std::isfinite(planeDistanceMm) &&
               icpConvergenceRelative > 0.0 && std::isfinite(icpConvergenceRelative) &&
               qemInversionThreshold > 0.0 && std::isfinite(qemInversionThreshold);
    }

    [[nodiscard]] bool operator==(const Tolerances& other) const = default;
};

inline constexpr Tolerances kDefaultTolerances{};

}
