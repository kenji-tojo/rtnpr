#include "raytracer.h"

#include <thread>

#include "rtnpr_math.hpp"
#include "linetest.hpp"
#include "brdf.hpp"
#include "pathtrace.hpp"
#include "thread.hpp"


namespace rtnpr {

void RayTracer::step_gui(
        Image<unsigned char, PixelFormat::RGB> &img,
        const Scene &scene,
        const Options &opts
) {
    step(img.width(), img.height(), img, scene, opts);
}

void RayTracer::step_headless(
        const int width,
        const int height,
        const Scene &scene,
        const Options &opts
) {
    int _dummy = -1;
    step<int, true>(width, height, _dummy, scene, opts);
}

template<typename Image_, bool headless>
void RayTracer::step(
        unsigned int width,
        unsigned int height,
        Image_ &img,
        const Scene &scene,
        const Options &opts
) {
    using namespace std;
    using namespace Eigen;

    if constexpr(headless) { static_assert(std::is_same_v<Image_, int>); }
    else { assert(width == img.width() && height == img.height()); }

    resize(width, height);

    if (opts.rt.spp_frame <= 0) { return; }
    if (m_spp > opts.rt.spp) { return; }

    assert(scene.camera);
    auto &camera = *scene.camera;

    const unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<Sampler<float>> sampler_pool(nthreads);
    std::vector<std::vector<Hit>> stencil_pool(nthreads);
    auto func0 = [&](int ih, int iw, int tid) {
        const int spp_frame = opts.rt.spp_frame;

        Vector3f L{0.f,0.f,0.f};
        float alpha_fore = 0.f;
        float alpha_line = 0.f;

        for (int ii = 0; ii < spp_frame; ++ii) {
            const auto [cen_w,cen_h] = sample_pixel(
                    (float(iw)+.5f)/float(width),
                    (float(ih)+.5f)/float(height),
                    1.f/float(width), 1.f/float(height),
                    sampler_pool[tid]
            );

            const float weight = 1.f / float(spp_frame);

            auto &stencil = stencil_pool[tid];
            stencil.clear();
            stencil.resize(opts.flr.n_aux+1);
            Hit hit;
            Ray ray = camera.spawn_ray(cen_w, cen_h);
            scene.ray_cast(ray, hit);

            stencil[0] = hit;
            float line_weight = 0.f;
            if (opts.flr.enable) {
                line_weight = stencil_test(
                        camera, cen_w, cen_h,
                        opts.flr.width/800.f,
                        scene, stencil,
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

        accumulate_sample(/*pix_id=*/ih*width+iw, L, alpha_fore, alpha_line, opts);
        if constexpr(!headless) { composite(iw, ih, img, opts); }
    };
    parallel_for(width, height, func0, nthreads);

    m_spp += opts.rt.spp_frame;
}

void RayTracer::screenshot(
        Image<float, PixelFormat::RGBA> &img,
        const Options &opts
) {
    if (img.width() != m_width || img.height() != m_height) {
        img.resize(m_width, m_height);
    }
    for (int iw = 0; iw < m_width; ++iw) {
        for (int ih = 0; ih < m_height; ++ih) {
            composite(iw, ih, img, opts);
        }
    }
}

template<typename Image_>
void RayTracer::composite(
        unsigned int iw,
        unsigned int ih,
        Image_ &img,
        const Options &opts
) {
    using namespace Eigen;

    const unsigned int pix_id = ih*img.width()+iw;
    const float alpha_line = opts.flr.enable ? m_alpha_line[pix_id] : 0.f;

    // foreground
    Vector3f c = opts.tone.mapper.map3(m_foreground[pix_id], opts.tone.theme_id);
    if (opts.flr.line_only) { c = Vector3f::Ones(); }
    c *= math::max(0.f, m_alpha_fore[pix_id]-alpha_line);

    // line
    Vector3f line_color = opts.flr.line_color;
    if (opts.tone.map_lines) { line_color = opts.tone.mapper.map(5.f*alpha_line, opts.tone.theme_id); }
    c += alpha_line * line_color;

    // background
    const float alpha = math::max(m_alpha_fore[pix_id], alpha_line);

    if constexpr(std::is_same_v<typename Image_::dtype, unsigned char>) {
        c += math::max(0.f, 1.f-alpha) * opts.rt.back_color;
        img(iw,ih,0) = math::to_u8(c[0]);
        img(iw,ih,1) = math::to_u8(c[1]);
        img(iw,ih,2) = math::to_u8(c[2]);
    }
    else {
        static_assert(std::is_floating_point_v<typename Image_::dtype>);
        static_assert(Image_::fmt == PixelFormat::RGBA);
        const unsigned int ih_flipped = img.height()-1-ih;
        math::clip3(c,0.f,1.f);
        img(iw,ih_flipped,0) = c[0];
        img(iw,ih_flipped,1) = c[1];
        img(iw,ih_flipped,2) = c[2];
        img(iw,ih_flipped,3) = math::clip(alpha, 0.f, 1.f);
    }
}

void RayTracer::resize(unsigned int width, unsigned int height) {
    if (m_width == width && m_height == height) { return; }
    m_width = width;
    m_height = height;
    const unsigned int pixels = m_width * m_height;
    m_foreground.clear();
    m_foreground.resize(pixels,Eigen::Vector3f::Zero());
    m_alpha_fore.clear();
    m_alpha_fore.resize(pixels,0.);
    m_alpha_line.clear();
    m_alpha_line.resize(pixels,0.);
    m_spp = 0;
}

} // namespace rtnpr