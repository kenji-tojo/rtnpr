#pragma once

#include <memory>
#include <vector>

#include "options.hpp"
#include "scene.hpp"
#include "sampler.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "pathtrace.hpp"
#include "linetest.hpp"
#include "thread.hpp"


namespace rtnpr {

class RayTracer {
public:
    [[nodiscard]] unsigned int spp() const { return m_spp; }

    void reset() {
        const auto width  = m_color_rgb.shape(0);
        const auto height = m_color_rgb.shape(1);
        resize(width,height,/*force_run=*/true);
    }

    template<bool headless_, typename Image_>
    void step(Image_ &img, const Scene &scene, const Options &opts);

    template<bool flip_axis_, typename Image_>
    void write(unsigned int iw, unsigned int ih, Image_ &img, const Options &opts);

    template<bool flip_axis_, typename Image_>
    void write(Image_ &img, const Options &opts) {
        const auto width  = m_color_rgb.shape(0);
        const auto height = m_color_rgb.shape(1);

        if (img.shape(0)!=width || img.shape(1)!=height)
            return;

        for (int iw = 0; iw < width; ++iw)
            for (int ih = 0; ih < height; ++ih)
                write<flip_axis_>(iw, ih, img, opts);
    }

private:
    Image<float, /*channels=*/3> m_color_rgb;
    Image<float, /*channels=*/1> m_alpha;
    Image<float, /*channels=*/1> m_alpha_line;

    unsigned int m_spp = 0;

    void resize(unsigned int width, unsigned int height, bool force_run = false) {
        if (!force_run && m_color_rgb.shape(0)==width && m_color_rgb.shape(1)==height)
            return;
        m_color_rgb .resize(width, height);
        m_alpha     .resize(width, height);
        m_alpha_line.resize(width, height);
        m_color_rgb .clear();
        m_alpha     .clear();
        m_alpha_line.clear();
        m_spp = 0;
    }

    void accumulate_sample(
            unsigned int iw, unsigned int ih,
            Eigen::Vector3f &rgb, float alpha, float alpha_line,
            unsigned int spp
    ) {
        float t = float(m_spp) / float(m_spp + spp);

        for (int ic = 0; ic < 3; ++ic)
            m_color_rgb(iw,ih,ic) = t * m_color_rgb(iw,ih,ic) + (1.f-t) * rgb[ic];

        m_alpha(iw,ih) = t * m_alpha(iw,ih) + (1.f-t) * alpha;
        m_alpha_line(iw,ih) = t * m_alpha_line(iw,ih) + (1.f-t) * alpha_line;
    }
};


template<bool flip_axis_, typename Image_>
void RayTracer::write(unsigned int iw, unsigned int ih, Image_ &img, const Options &opts) {
    using namespace Eigen;

    float alpha = opts.flr.enable && opts.flr.line_only ? 0.f : m_alpha(iw,ih);
    float alpha_line = opts.flr.enable ? m_alpha_line(iw, ih) : 0.f;
    alpha_line = math::clip(alpha_line, 0.f, 1.f);
    alpha += alpha_line;
    alpha = math::clip(alpha, 0.f, 1.f);

    // foreground
    auto rgb = Vector3f{m_color_rgb(iw,ih,0),m_color_rgb(iw,ih,1),m_color_rgb(iw,ih,2)};
    rgb = opts.tone.mapper.map3(rgb, opts.tone.theme_id);

    // line
    Vector3f line_color = opts.flr.line_color;
    if (opts.tone.map_lines)
        line_color = opts.tone.mapper.map(5.f*alpha_line, opts.tone.theme_id);
    rgb += alpha_line * line_color;

    if constexpr(std::is_same_v<typename Image_::Scalar, unsigned char>) {
        rgb += math::max(0.f, 1.f-alpha) * opts.rt.back_color;
        if constexpr(flip_axis_) std::swap(iw,ih);
        img(iw,ih,0) = math::to_u8(rgb[0]);
        img(iw,ih,1) = math::to_u8(rgb[1]);
        img(iw,ih,2) = math::to_u8(rgb[2]);
    }
    else {
        static_assert(std::is_floating_point_v<typename Image_::Scalar>);
        assert(img.shape(2)==4);
        ih = img.shape(1)-ih-1;
        if constexpr(flip_axis_) std::swap(iw,ih);
        if (opts.tone.mapper.mode == ToneMapper::Raw) {
            float back = math::max(0.f, 1.f-alpha);
            img(iw, ih, 0) = rgb[0] + back;
            img(iw, ih, 1) = rgb[1] + back;
            img(iw, ih, 2) = rgb[2] + back;
            img(iw, ih, 3) = 1.f;
        }
        else {
            math::clip3(rgb, 0.f, 1.f);
            img(iw, ih, 0) = rgb[0];
            img(iw, ih, 1) = rgb[1];
            img(iw, ih, 2) = rgb[2];
            img(iw, ih, 3) = alpha;
        }
    }
}


template<bool write_image, typename Image_>
void RayTracer::step(Image_ &img, const Scene &scene, const Options &opts) {
    using namespace std;
    using namespace Eigen;

    const auto width  = img.shape(0);
    const auto height = img.shape(1);

    resize(width, height);

    if (opts.rt.spp_frame <= 0) { return; }
    if (m_spp > opts.rt.spp) { return; }

    if (!scene.camera) { return; }
    auto &camera = *scene.camera;

    const unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<Sampler<float>> sampler_pool(nthreads);
    std::vector<std::vector<Hit>> stencil_pool(nthreads);

    auto func0 = [&](int ih, int iw, int tid) {
        const unsigned int spp_frame = opts.rt.spp_frame;

        Vector3f rgb{0.f,0.f,0.f};
        float alpha = 0.f;
        float alpha_line = 0.f;

        for (int ii = 0; ii < spp_frame; ++ii) {
            const auto [cen_w,cen_h] = sample_pixel(
                    (float(iw)+.5f)/float(width),
                    (float(ih)+.5f)/float(height),
                    1.f/float(width), 1.f/float(height),
                    sampler_pool[tid]
            );

            const float weight = 1.f/float(spp_frame);

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
                alpha_line += weight * math::min(1.f, line_weight);
            }

            if (hit.obj_id >= 0 && line_weight == 0.f) {
                kernel::ptrace(
                        ray, hit, scene,
                        weight, rgb,
                        opts,
                        sampler_pool[tid]
                );
                assert(!std::isnan(rgb.sum()));
                alpha += weight;
            }
        }

        accumulate_sample(iw, ih, rgb, alpha, alpha_line, spp_frame);

        if constexpr(write_image)
            write</*flip_axis=*/false>(iw, ih, img, opts);
    };

    parallel_for(width, height, func0, nthreads);

    m_spp += opts.rt.spp_frame;
}


} // namespace rtnpr