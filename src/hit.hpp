#pragma once

#include <limits>

#include <Eigen/Dense>

namespace rtnpr {

struct Hit {
public:
    float dist = std::numeric_limits<float>::max();
    Eigen::Vector3f pos = Eigen::Vector3f::Zero();
    Eigen::Vector3f nrm = Eigen::Vector3f::UnitZ();
    int obj_id = -1;
    int mat_id = -1;
    int tri_id = 0;
};



} // namespace rtnpr