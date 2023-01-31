#pragma once

#include <memory>
#include <vector>

#include "rtnpr_math.hpp"
#include "brdf.hpp"
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

    float brdf_val, pdf;

    for (int dd = 0; dd < opts.rt.depth-1; ++dd)
    {
        brdf.sample_dir(nrm, wo, wi, pdf, brdf_val, sampler);
        if (brdf_val <= 0) { return; }
        assert(pdf > 0);

        weight *= brdf_val / pdf;

        Hit hit;
        Ray ray{pos,wi};

        scene.ray_cast(ray,hit);

        if (hit.obj_id < 0) {
            L = float(wi.z()>0) * weight;
//            L = weight;
//            L = weight * math::max(0.f, wi.z()) * 2.f;
            return;
        }

        pos = hit.pos;
        nrm = hit.nrm;
        wo = -wi;
    }
}

} // namespace
} // namespace rtnpr::kernel