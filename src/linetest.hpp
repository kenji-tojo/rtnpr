#pragma once

#include <vector>

#include "rtnpr_math.hpp"
#include "hit.hpp"
#include "options.hpp"
#include "sampler.hpp"
#include "ray.hpp"
#include "scene.hpp"

namespace rtnpr {

template<typename T>
void sample_stencil(
        const T inv_mvp[16],
        T cen_w, T cen_h, T radius,
        int n_aux,
        const Scene &scene,
        std::vector<Hit> &stencil,
        UniformDiscSampler<T> &sampler
) {
    cen_w = math::clip<T>(cen_w, 0, 1);
    cen_h = math::clip<T>(cen_h, 0, 1);

    stencil.clear();
    stencil.resize(n_aux+1);
    {
        Ray ray(inv_mvp, cen_w, cen_h);
        scene.ray_cast(ray, stencil[0]);
    }

    for (int ii = 0; ii < n_aux; ++ii)
    {
        auto [d_w, d_h] = sampler.sample();
        d_w *= radius;
        d_h *= radius;

        Ray ray(inv_mvp, cen_w+d_w, cen_h+d_h);
        scene.ray_cast(ray, stencil[ii+1]);
    }
}

bool test_feature_line(
        const std::vector<Hit> &stencil,
        const Options &opts
) {
    if (stencil.empty()) { return false; }

    const auto &cen_hit = stencil[0];
    for (int ii = 1; ii < stencil.size(); ++ii)
    {
        const auto &aux_hit = stencil[ii];
        if (cen_hit.obj_id != aux_hit.obj_id ||
            cen_hit.tri_id != aux_hit.tri_id)
        {
            return  true;
        }
    }
    return false;
}

} // namespace rtnpr