#include "raytracer.h"

#include <thread>

#include "rtnpr_math.hpp"
#include "linetest.hpp"
#include "brdf.hpp"
#include "pathtrace.hpp"
#include "thread.hpp"


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
    if (m_foreground.size() != img.size()) {
        m_foreground.resize(img.size());
        reset();
    }

    unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<UniformSampler<float>> sampler_pool(nthreads);
    std::vector<std::vector<Hit>> stencil(nthreads);
    auto func0 = [&](int ih, int iw, int tid) {
        const int spp_frame = opts.rt.spp_frame;
        if (spp_frame <= 0) { return; }
        if (m_spp > opts.rt.spp) { return; }

        Vector3f L{0.f,0.f,0.f};
        float alpha_fore = 0.f;
        float alpha_line = 0.f;

        for (int ii = 0; ii < spp_frame; ++ii)
        {
            const auto [cen_w,cen_h] = sample_pixel(
                    (float(iw)+.5f)/float(width),
                    (float(ih)+.5f)/float(height),
                    1.2f/float(width), 1.2f/float(height),
                    sampler_pool[tid]
            );

            const float weight = 1.f / float(spp_frame);

            auto &stncl = stencil[tid];
            stncl.clear();
            stncl.resize(opts.flr.n_aux+1);
            Hit hit;
            Ray ray = camera.spawn_ray(cen_w, cen_h);
            scene.ray_cast(ray, hit);

            stncl[0] = hit;
            float line_weight = 0.f;
            if (opts.flr.enable) {
                line_weight = stencil_test(
                        camera, cen_w, cen_h,
                        opts.flr.linewidth/800.f,
                        scene, stncl,
                        sampler_pool[tid], opts
                );
                line_weight = math::min(1.f, line_weight);
                alpha_line += weight * line_weight;
            }

            if (hit.obj_id >= 0) {
                kernel::ptrace(
                        ray, hit, scene,
                        weight, L,
                        opts,
                        sampler_pool[tid]
                );
                assert(!std::isnan(L.sum()));
                alpha_fore += weight;
            }
        }

        auto pix_id = ih*width+iw;
        accumulate_sample(img, pix_id, L, alpha_fore, alpha_line, opts);
        if (!opts.headless) { composite(img, pix_id, opts); }
    };
    parallel_for(width, height, func0, nthreads);

    m_spp += opts.rt.spp_frame;
}

void RayTracer::accumulate_sample(
        std::vector<unsigned char> &img,
        unsigned int pix_id,
        Eigen::Vector3f &L,
        float alpha_fore,
        float alpha_line,
        const Options &opts
) {
    float t = float(m_spp) / float(m_spp + opts.rt.spp_frame);
    m_foreground[pix_id] = t * m_foreground[pix_id] + (1.f-t) * L;
    m_alpha_fore[pix_id] = t * m_alpha_fore[pix_id] + (1.f-t) * alpha_fore;
    m_alpha_line[pix_id] = t * m_alpha_line[pix_id] + (1.f-t) * alpha_line;
}

template<typename Scalar>
void RayTracer::composite(
        std::vector<Scalar> &img,
        unsigned int pix_id,
        const Options &opts
) {
    using namespace Eigen;
    const float alpha_line = opts.flr.enable ? m_alpha_line[pix_id] : 0.f;

    // foreground
    Vector3f c = opts.tone.mapper.map3(m_foreground[pix_id], opts.tone.map_mode);
    if (opts.flr.line_only) { c = Vector3f::Ones(); }
    c *= math::max(0.f, m_alpha_fore[pix_id]-alpha_line);

    // line
    Vector3f line_color = opts.flr.line_color;
    if (opts.tone.map_lines) { line_color = opts.tone.mapper.map(5.f*alpha_line); }
    c += alpha_line * line_color;

    // background
    float alpha = math::max(m_alpha_fore[pix_id], alpha_line);
    c += math::max(0.f, 1.f-alpha) * opts.rt.back_color;

    if constexpr(std::is_same_v<Scalar, unsigned char>) {
        img[pix_id*3+0] = math::to_u8(c[0]);
        img[pix_id*3+1] = math::to_u8(c[1]);
        img[pix_id*3+2] = math::to_u8(c[2]);
    }
    else {
        static_assert(std::is_floating_point_v<Scalar>);
        math::clip3(c,0,1);
        img[pix_id*3+0] = c[0];
        img[pix_id*3+1] = c[1];
        img[pix_id*3+2] = c[2];
    }
}

void RayTracer::reset()
{
    auto size = m_foreground.size();
    m_foreground.clear();
    m_foreground.resize(size,Eigen::Vector3f::Zero());
    m_alpha_fore.clear();
    m_alpha_fore.resize(size/3,0.);
    m_alpha_line.clear();
    m_alpha_line.resize(size/3,0.);
    m_spp = 0;
}

} // namespace rtnpr