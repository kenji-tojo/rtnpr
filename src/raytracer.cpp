#include "raytracer.h"

#include <thread>

#include "delfem2/thread.h"

#include "rtnpr_math.hpp"
#include "linetest.hpp"
#include "brdf.hpp"
#include "pathtrace.hpp"


namespace dfm2 = delfem2;

namespace rtnpr {

void RayTracer::step(
        std::vector<unsigned char> &img,
        unsigned int width, unsigned int height,
        const Camera &camera,
        const Options &opts
) {
    using namespace std;
    using namespace Eigen;

    img.resize(height*width*3);
    if (m_img.size() != img.size()) {
        m_img.resize(img.size());
        reset();
    }

    const Vector3f line_color{
            50.f/255.f, 50.f/255.f, 50.f/255.f
    };

    unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<UniformSampler<float>> sampler_pool(nthreads);
    std::vector<std::vector<Hit>> stencils(nthreads);
    auto func0 = [&](int ih, int iw, int tid) {
        const int spp = opts.rt.spp;
        if (spp <= 0) { return; }
        if (m_spp_total + spp > opts.rt.spp_max) { return; }

        Vector3f L{0.f,0.f,0.f};
        float weight = 1.f / float(spp);
        const auto light_dir = Vector3f::UnitZ();

        for (int ii = 0; ii < spp; ++ii)
        {
            const auto [cen_w,cen_h] = sample_pixel(
                    (float(iw)+.5f)/float(width),
                    (float(ih)+.5f)/float(height),
                    1.2f/float(width), 1.2f/float(height),
                    sampler_pool[tid]
            );

            auto &stncl = stencils[tid];
            stncl.clear();
            stncl.resize(opts.flr.n_aux+1);
            auto &hit = stncl[0];
            Ray ray = camera.spawn_ray(cen_w, cen_h);
            scene.ray_cast(ray, hit);

            sample_stencil(
                    camera, cen_w, cen_h,
                    opts.flr.linewidth/800.f,
                    scene, stncl,
                    sampler_pool[tid]
            );

            Vector3f L_single{1.f,1.f,1.f};
            if (hit.obj_id >= 0) {
                float c;
                kernel::ambient_occlusion(
                        ray, hit,
                        scene, weight,
                        c, opts,
                        sampler_pool[tid]
                );
                L_single *= c;
            }
            if (test_feature_line(stncl,opts)) {
                L_single = line_color;
            }
            L += weight * L_single;
        }

        accumulate_and_write(img, ih*width+iw, L, spp);
    };
    delfem2::parallel_for(width, height, func0, nthreads);

    m_spp_total += opts.rt.spp;
}

void RayTracer::accumulate_and_write(
        std::vector<unsigned char> &img,
        unsigned int pix_id,
        Eigen::Vector3f &L, int spp
) {
    float t = float(m_spp_total) / float(m_spp_total + spp);
    for (int ii = 0; ii < 3; ++ii) {
        auto kk = 3*pix_id+ii;
        m_img[kk] = t * m_img[kk] + (1.f-t) * L[ii];
        img[kk] = math::to_u8(m_img[kk]);
    }
}

void RayTracer::reset()
{
    auto size = m_img.size();
    m_img.clear();
    m_img.resize(size,0.);
    m_spp_total = 0;
}

} // namespace rtnpr