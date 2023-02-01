#pragma once

#include <memory>
#include <vector>

#include "rtnpr_math.hpp"
#include "brdf.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "options.hpp"

namespace rtnpr::kernel {
namespace {

void ambient_occlusion(
        const Ray &first_ray,
        const Hit &first_hit,
        const Scene &scene,
        float weight,
        float &L,
        const Options &opts,
        UniformSampler<float> &sampler
) {
    using namespace Eigen;

    L = 0;
    if (first_hit.obj_id < 0) { return; }

    BRDF brdf;
    Vector3f pos, nrm, wo, wi;
    pos = first_hit.pos;
    nrm = first_hit.nrm;
    wo = -first_ray.dir;

    float pdf = 1.f;

    for (int dd = 0; dd < opts.rt.depth-1; ++dd)
    {
        float brdf_val;
        brdf.sample_dir(nrm, wo, wi, brdf_val, sampler);
        pdf *= brdf.pdf(nrm, wo, wi);
        if (brdf_val <= 0) { return; }
        weight *= brdf_val;

        Hit hit;
        Ray ray{pos,wi};

        scene.ray_cast(ray,hit);

        if (hit.obj_id < 0) {
            if (pdf <= 0) { return; }
            weight /= pdf;
//            L = float(wi.z()>0) * weight;
//            L = weight;
            L = weight * math::max(0.f, wi.z()) * 3.f;
            return;
        }

        pos = hit.pos;
        nrm = hit.nrm;
        wo = -wi;
    }
}

} // namespace
} // namespace rtnpr::kernel