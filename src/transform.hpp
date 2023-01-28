#pragma once

#include "rtnpr_math.hpp"

#include <Eigen/Geometry>

namespace rtnpr {

struct Transform {
public:
    float scale = 1.f;
    Eigen::Vector3f angle_axis = Eigen::Vector3f::Zero();
    Eigen::Vector3f shift = Eigen::Vector3f::Zero();
    [[nodiscard]] Eigen::Matrix3f rot() const {
        using namespace Eigen;
        Matrix3f m;
        m = AngleAxisf(angle_axis[0], Vector3f::UnitX())
            * AngleAxisf(angle_axis[1], Vector3f::UnitY())
            * AngleAxisf(angle_axis[2], Vector3f::UnitZ());
        return m;
    }
};

} // namespace rtnpr