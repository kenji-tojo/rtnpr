#pragma once

#include <limits>

#include <Eigen/Dense>

namespace rtnpr {

struct Ray{
public:
    Ray(Eigen::Vector3f &&_org, Eigen::Vector3f &&_dir)
    {
       org = _org;
       dir = _dir;
    }

    Eigen::Vector3f org;
    Eigen::Vector3f dir;
    float tmin = 0.f;
    float tmax = std::numeric_limits<float>::max();
};

} // rtnpr