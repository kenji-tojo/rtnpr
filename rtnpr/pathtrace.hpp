#pragma once

#include <memory>
#include <vector>

#include "rtnpr_math.hpp"
#include "brdf.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "options.hpp"
#include "raystencil.hpp"

namespace rtnpr::kernel {
namespace {

bool ptrace(
        Ray ray,
        RayStencil &stc,
        const Scene &scene,
        float weight,
        Eigen::Vector3f &contrib,
        const Options &opts,
        Sampler<float> &sampler
) {
    using namespace Eigen;

    const float initial_weight = weight;

    if (stc.detect_line(opts)) {
        contrib += initial_weight * opts.flr.line_color;
        return true;
    }

    if (!stc.prim_hit())
        return false;

    if (scene.object(stc.prim_hit().obj_id).transparent)
        return false;

    Vector3f pos, nrm, wo, wi;
    pos = stc.prim_hit().pos;
    nrm = stc.prim_hit().nrm;
    wo = -ray.dir;
    int mat_id = stc.prim_hit().mat_id;

    float brdf_val;
    float pdf_val;

    const bool line_only = opts.flr.enable && opts.flr.line_only;

    for (int dd = 0; dd < opts.rt.depth-1; ++dd) {
        if (scene.light && !line_only) {
            scene.light->sample_dir(wi, sampler);
            Hit hit;
            ray = Ray{pos,wi};
            scene.ray_cast(ray,hit);

            if (!hit) {
                assert(mat_id < scene.brdf.size());
                assert(scene.brdf[mat_id]);
                brdf_val = scene.brdf[mat_id]->eval(nrm, wo, wi);
                pdf_val  = scene.light->pdf(wi);
                if (brdf_val > 0) {
                    assert(pdf_val > 0);
                    Vector3f c = weight * brdf_val * scene.light->Le(wi) / pdf_val;
                    math::clip3(c, 0.f, 1e1f); // energy clipping to remove fireflies
                    contrib += c;
                }
            }
        }

        assert(mat_id < scene.brdf.size());
        assert(scene.brdf[mat_id]);
        scene.brdf[mat_id]->sample_dir(nrm, wo, wi, brdf_val, pdf_val, sampler);
        if (brdf_val <= 0) break;
        assert(pdf_val > 0);
        weight *= brdf_val / pdf_val;

        Hit hit;
        ray = Ray{pos,wi};
        scene.ray_cast(ray,hit);

        if (dd == 0 && stc.is_reflecting(scene, opts)) {
            const auto &ph = stc.prim_hit();
            stc.reflect_aux(scene, /*org_mirrored=*/ph.pos - ph.dist * wi);
            stc.prim_hit() = hit;
            if (stc.detect_line(opts)) {
                contrib += initial_weight * opts.flr.line_color;
                return true;
            }
        }

        if (!hit) break; // exit from the scene

        pos = hit.pos;
        nrm = hit.nrm;
        wo = -wi;
        mat_id = hit.mat_id;
    }

    return !line_only;
}

} // namespace
} // namespace rtnpr::kernel