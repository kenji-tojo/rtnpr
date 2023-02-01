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

    unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<UniformSampler<float>> sampler_pool(nthreads);
    std::vector<std::vector<Hit>> stencils(nthreads);
    auto func0 = [&](int ih, int iw, int tid) {
        const int spp_frame = opts.rt.spp_frame;
        if (spp_frame <= 0) { return; }
        if (m_spp > opts.rt.spp) { return; }

        Vector3f L{0.f,0.f,0.f};
        float alpha = 0.f;
        float alpha_line = 0.f;

        for (int ii = 0; ii < spp_frame; ++ii)
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

            const float weight = 1.f / float(spp_frame);

            if (hit.obj_id >= 0) {
                kernel::ambient_occlusion(
                        ray, hit, scene,
                        L, weight,
                        opts,
                        sampler_pool[tid]
                );
            }
            if (test_feature_line(stncl,opts)) {
                alpha_line += weight;
            }
            else if (hit.obj_id >= 0) {
                alpha += weight;
            }
        }
        accumulate_and_write(img, ih*width+iw, L, alpha, alpha_line, opts);
    };
    delfem2::parallel_for(width, height, func0, nthreads);

    m_spp += opts.rt.spp_frame;
}

void RayTracer::accumulate_and_write(
        std::vector<unsigned char> &img,
        unsigned int pix_id,
        Eigen::Vector3f &L,
        float alpha, float alpha_line,
        const Options &opts
) {
    double t = double(m_spp) / double(m_spp + opts.rt.spp_frame);
    m_alpha[pix_id] = t * m_alpha[pix_id] + (1.-t) * alpha;
    m_alpha_line[pix_id] = t * m_alpha_line[pix_id] + (1.-t) * alpha_line;
    for (int ii = 0; ii < 3; ++ii) {
        auto kk = 3*pix_id+ii;
        m_img[kk] = t * m_img[kk] + (1.-t) * double(L[ii]);

        double c = math::tone_map_Reinhard(m_img[kk], 4.);
        c *= m_alpha[pix_id];
        c += m_alpha_line[pix_id] * opts.flr.line_color[ii];
        c += math::max(0., 1.-m_alpha[pix_id]-m_alpha_line[pix_id]);
        img[kk] = math::to_u8(c);
    }
}

void RayTracer::reset()
{
    auto size = m_img.size();
    m_img.clear();
    m_img.resize(size,0.);
    m_alpha.clear();
    m_alpha.resize(size/3,0.);
    m_alpha_line.clear();
    m_alpha_line.resize(size/3,0.);
    m_spp = 0;
}

} // namespace rtnpr