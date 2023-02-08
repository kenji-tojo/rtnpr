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

void ptrace(
        const Ray &first_ray,
        const Hit &first_hit,
        const Scene &scene,
        float weight,
        Eigen::Vector3f &L,
        const Options &opts,
        UniformSampler<float> &sampler
) {
    using namespace Eigen;

    if (first_hit.obj_id < 0) { return; }
    if (scene.brdf.empty()) { return; }
    if (!scene.light) { return; }

    Vector3f pos, nrm, wo, wi;
    pos = first_hit.pos;
    nrm = first_hit.nrm;
    wo = -first_ray.dir;
    int mat_id = first_hit.mat_id;

    for (int dd = 0; dd < opts.rt.depth-1; ++dd)
    {
        assert(mat_id < scene.brdf.size());
        float brdf_val;
        {
            scene.light->sample_dir(wi, sampler);
            Hit hit;
            Ray ray{pos,wi};
            scene.ray_cast(ray,hit);
            if (hit.obj_id < 0) {
                brdf_val = scene.brdf[mat_id]->eval(nrm, wo, wi);
                float pdf = scene.light->pdf(wi);
                if (brdf_val > 0) {
                    assert(pdf > 0);
                    Vector3f contrib = weight * brdf_val * scene.light->Le(wi) / pdf;
                    // energy clipping to remove fireflies
                    math::clip3(contrib, 0.f, 1e1f);
                    L += contrib;
                }
            }
        }

        scene.brdf[mat_id]->sample_dir(nrm, wo, wi, brdf_val, sampler);
        float pdf = scene.brdf[mat_id]->pdf(nrm, wo, wi);
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