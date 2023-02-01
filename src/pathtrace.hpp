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
        float &L,
        const Options &opts,
        UniformSampler<float> &sampler
) {
    using namespace Eigen;

    L = 0;
    if (first_hit.obj_id < 0) { return; }
    if (opts.scene.brdf.empty()) { return; }
    if (opts.scene.light.empty()) { return; }

    const auto &brdf = opts.scene.brdf;
    const auto &light = opts.scene.light[0];

    Vector3f pos, nrm, wo, wi;
    pos = first_hit.pos;
    nrm = first_hit.nrm;
    wo = -first_ray.dir;
    int mat_id = first_hit.mat_id;

    float weight = 1.f;

    for (int dd = 0; dd < opts.rt.depth-1; ++dd)
    {
        assert(mat_id < brdf.size());
        float brdf_val;
        {
            light->sample_dir(wi, sampler);
            Hit hit;
            Ray ray{pos,wi};
            scene.ray_cast(ray,hit);
            if (hit.obj_id < 0) {
                brdf_val = brdf[mat_id]->eval(nrm, wo, wi);
                if (brdf_val > 0) {
                    L += weight * brdf_val * light->Le(wi) / light->pdf(wi);
                }
            }
        }

        brdf[mat_id]->sample_dir(nrm, wo, wi, brdf_val, sampler);
        float pdf = brdf[mat_id]->pdf(nrm, wo, wi);
        if (brdf_val <= 0) { return; }
        assert(pdf > 0);
        weight *= brdf_val / pdf;

        Hit hit;
        Ray ray{pos,wi};
        scene.ray_cast(ray,hit);
        if (hit.obj_id < 0) { return; }

        pos = hit.pos;
        nrm = hit.nrm;
        wo = -wi;
        mat_id = hit.mat_id;
    }
}

} // namespace
} // namespace rtnpr::kernel