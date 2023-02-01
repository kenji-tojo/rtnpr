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
        const Camera &camera,
        T cen_w, T cen_h, T radius,
        const Scene &scene,
        std::vector<Hit> &stencil,
        std::vector<Hit> &stencil_higher,
        UniformSampler<T> &sampler,
        const Options &opts
) {
    for (int ii = 1; ii < stencil.size(); ++ii)
    {
        auto [d_w, d_h] = sample_disc(sampler);
        d_w *= radius;
        d_h *= radius;

        auto ray = camera.spawn_ray(cen_w+d_w, cen_h+d_h);
        scene.ray_cast(ray, stencil[ii]);
    }

    using namespace Eigen;
    Vector3f wi;
    stencil_higher.resize(stencil.size());
    for (int ii = 0; ii < stencil.size(); ++ii) {
        stencil_higher[ii] = stencil[ii];
        auto &hit = stencil_higher[ii];
        for (int dd = 0; dd < 3; ++dd) {
            if (hit.obj_id < 0) { break; }
            const auto &brdf = opts.scene.brdf[hit.mat_id];
            if (!brdf->reflect_line) { break; }
            float brdf_val;
            brdf->sample_dir(hit.nrm, hit.wo, wi, brdf_val, sampler);
            Ray ray{hit.pos, wi};
            Hit _hit;
            scene.ray_cast(ray, _hit);
            hit = _hit;
        }
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
        if (cen_hit.obj_id != aux_hit.obj_id) { return true; }

        if (opts.flr.normal) {
            const auto &n0 = cen_hit.nrm;
            const auto &n1 = aux_hit.nrm;
            if (std::acos(n0.dot(n1)) > .2*M_PI) { return true; }
        }

        if (opts.flr.position) {
            const auto &p0 = cen_hit.pos;
            const auto &p1 = aux_hit.pos;
            if ((p0-p1).norm() > 1e-1f) { return true; }
        }

        if (opts.flr.wireframe) {
            if (cen_hit.prim_id != aux_hit.prim_id) { return true; }
        }
    }
    return false;
}

} // namespace rtnpr