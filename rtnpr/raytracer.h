#pragma once

#include <memory>
#include <vector>

#include "options.hpp"
#include "scene.hpp"
#include "sampler.hpp"
#include "camera.hpp"
#include "image.hpp"


namespace rtnpr {

class RayTracer {
public:
    Scene scene;

    void step_gui(
            Image<unsigned char, PixelFormat::RGB> &img,
            const Camera &camera,
            const Options &opts
    );

    void reset();
private:
    std::vector<Eigen::Vector3f> m_foreground;
    std::vector<float> m_alpha_fore;
    std::vector<float> m_alpha_line;

    unsigned int m_spp = 0;

    inline void accumulate_sample(
            unsigned int pix_id,
            Eigen::Vector3f &L,
            float alpha_fore, float alpha_line,
            const Options &opts
    ) {
        float t = float(m_spp) / float(m_spp + opts.rt.spp_frame);
        m_foreground[pix_id] = t * m_foreground[pix_id] + (1.f-t) * L;
        m_alpha_fore[pix_id] = t * m_alpha_fore[pix_id] + (1.f-t) * alpha_fore;
        m_alpha_line[pix_id] = t * m_alpha_line[pix_id] + (1.f-t) * alpha_line;
    }

    template<typename Image_>
    void composite(
            unsigned int iw,
            unsigned int ih,
            Image_ &img,
            const Options &opts
    );

    template<typename Image_>
    void step(
            Image_ &img,
            const Camera &camera,
            const Options &opts
    );
};

} // namespace rtnpr