#pragma once

#include <vector>

#include "rtnpr_math.hpp"
#include "hit.hpp"
#include "options.hpp"
#include "sampler.hpp"
#include "ray.hpp"
#include "scene.hpp"

namespace rtnpr {
namespace {

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

bool all_reflected(
        const std::vector<Hit> &stencil,
        const Options &opts
) {
    for (const auto &hit: stencil) {
        if (hit.obj_id < 0) { return false; }
        if (!opts.scene.brdf[hit.mat_id]->reflect_line) { return false; }
    }
    return true;
}

float stencil_test(
        const Camera &camera,
        float cen_w, float cen_h, float radius,
        const Scene &scene,
        std::vector<Hit> &stencil,
        UniformSampler<float> &sampler,
        const Options &opts
) {
    float weight = 1.f;

    for (int ii = 1; ii < stencil.size(); ++ii) {
        auto [d_w, d_h] = sample_disc(sampler);
        d_w *= radius;
        d_h *= radius;
        auto ray = camera.spawn_ray(cen_w+d_w, cen_h+d_h);
        scene.ray_cast(ray, stencil[ii]);
    }

    if (test_feature_line(stencil, opts)) { return weight; }
    if (!all_reflected(stencil, opts)) { return 0.f; }

    const auto &brdf = opts.scene.brdf;
    Eigen::Vector3f org, wi;
    float brdf_val, pdf;

    {
        auto &hit = stencil[0];
        brdf[hit.mat_id]->sample_dir(hit.nrm, hit.wo, wi, brdf_val, sampler);
        pdf = brdf[hit.mat_id]->pdf(hit.nrm, hit.wo, wi);
        org = hit.pos - hit.dist * wi;
        Ray ray{hit.pos,wi};
        Hit _hit;
        scene.ray_cast(ray,_hit);
        hit = _hit;
    }

    for (int ii=1; ii < stencil.size(); ++ii) {
        auto &hit = stencil[ii];
        Ray ray{hit.pos,(hit.pos-org).normalized()};
        Hit _hit;
        scene.ray_cast(ray,_hit);
        hit = _hit;
    }

    if (test_feature_line(stencil, opts)) {
        if (brdf_val <= 0) { return 0.f; }
        assert(pdf > 0);
        weight *= brdf_val / pdf;
        assert(weight < 1.f+1e-6f);
        return weight;
    }

    return 0.f;
}

} // namespace
} // namespace rtnpr