#pragma once

#include "hit.hpp"


namespace rtnpr::vis {

template<typename Color_>
void surface_normal(const Hit &hit, float weight, Color_ &rgb, float &alpha) {
    if (!hit) { return; }

    const auto &nrm = hit.nrm;
    rgb[0] += weight*.5f*math::clip(nrm[0]+1.f, 0.f, 1.f);
    rgb[1] += weight*.5f*math::clip(nrm[1]+1.f, 0.f, 1.f);
    rgb[2] += weight*.5f*math::clip(nrm[2]+1.f, 0.f, 1.f);
    alpha += weight;
}

} // namespace rtnpr::vis