#pragma once

#include "hit.hpp"


namespace rtnpr::kernel {
namespace {

bool surface_normal(const Hit &hit, const Scene &scene, float weight, Eigen::Vector3f &contrib) {
    if (!hit)
        return false;

    if (scene.object(hit.obj_id).transparent)
        return false;

    float lo = .4f;
    float hi = .95f;
    const auto &nrm = hit.nrm;

    for (int ic = 0; ic < 3; ++ic) {
        float u = math::clip(.5f*(nrm[ic]+1.f), 0.f, 1.f);
        contrib[ic] += weight * (lo+(hi-lo)*u);
    }

    return true;
}

} // namespace
} // namespace rtnpr::kernel