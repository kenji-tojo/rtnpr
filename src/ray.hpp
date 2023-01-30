#pragma once

#include <limits>

#include <Eigen/Dense>

namespace rtnpr {

struct Ray{
public:
    template<typename VEC3>
    Ray(const VEC3 &_org, const VEC3 &_dir)
    {
        using namespace Eigen;
        org = Vector3f(_org[0],_org[1],_org[2]);
        dir = Vector3f(_dir[0],_dir[1],_dir[2]);
    }

    Eigen::Vector3f org;
    Eigen::Vector3f dir;
    float tmin = 0.f;
    float tmax = std::numeric_limits<float>::max();
};

} // rtnpr