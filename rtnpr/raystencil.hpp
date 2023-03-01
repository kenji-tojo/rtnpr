#pragma once

#include <vector>

#include "rtnpr_math.hpp"
#include "hit.hpp"
#include "ray.hpp"
#include "scene.hpp"


namespace rtnpr {

class RayStencil {
public:
    RayStencil(): stencil{/*n=*/1} {}

    [[nodiscard]] bool is_prim_only() const { return stencil.size()<=1;}
    [[nodiscard]] const Hit &prim_hit() const { return stencil[0]; }
    Hit &prim_hit() { return stencil[0]; }


    [[nodiscard]] size_t n_aux() const { return stencil.size()-1; }
    Hit &aux_hit(unsigned int index) { return stencil[1+index]; }
    [[nodiscard]] const Hit &aux_hit(unsigned int index) const { return stencil[1+index]; }

    void resize(unsigned int n_aux) { stencil.resize(1+n_aux); }
    void clear() { resize(0); }

    void cast_aux(
            const Camera &camera,
            float cen_w, float cen_h, float radius,
            const Scene &scene,
            Sampler<float> &sampler
    ) {
        for (int ia = 0; ia < n_aux(); ++ia) {
            auto &hit = aux_hit(ia);
            hit = Hit{};
            auto [w,h] = sample_disc(sampler);
            w *= radius;
            h *= radius;
            auto ray = camera.spawn_ray(cen_w+w, cen_h+h);
            scene.ray_cast(ray, hit);
        }
    }

    void reflect_aux(const Scene &scene, const Eigen::Vector3f &org_mirrored) {
        for (int ia = 0; ia < n_aux(); ++ia) {
            auto &hit = aux_hit(ia);
            Ray ray{hit.pos, (hit.pos-org_mirrored).normalized()};
            hit = Hit{};
            scene.ray_cast(ray, hit);
        }
    }

    bool is_reflecting(const Scene &scene, const Options &options) {
        for (const auto &hit: stencil) {
            if (!hit || !scene.brdf[hit.mat_id]->reflect_line)
                return false;
        }
        return true;
    }

    [[nodiscard]] const Hit &nearest_hit() const {
        int index = 0;
        for (int ia = 0; ia < n_aux(); ++ia) {
            const auto &hit = aux_hit(ia);
            if (hit && hit.dist < stencil[index].dist)
                index = 1+ia;
        }
        return stencil[index];
    }

    [[nodiscard]] bool detect_line(const Options &options) const {
        if (is_prim_only())
            return false;

        for (int ia = 0; ia < n_aux(); ++ia) {
            const auto &hit = aux_hit(ia);

            if (hit.obj_id != prim_hit().obj_id)
                return true;

            if (options.flr.normal) {
                const auto &n0 = prim_hit().nrm;
                const auto &n1 = hit.nrm;
                if (std::acos(n0.dot(n1)) > .2*M_PI)
                    return true;
            }

            if (options.flr.position) {
                const auto &p0 = prim_hit().pos;
                const auto &p1 = hit.pos;
                if ((p0-p1).norm() > 1e-1f)
                    return true;
            }

            if (options.flr.wireframe) {
                if (hit.prim_id != prim_hit().prim_id)
                    return true;
            }
        }

        return false;
    }

private:
    std::vector<Hit> stencil;

};

}