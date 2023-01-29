#pragma once

#include <memory>
#include <vector>

#include "options.hpp"
#include "scene.hpp"
#include "sampler.hpp"

namespace rtnpr {

class RayTracer {
public:
    Scene scene;

    void step(
            std::vector<unsigned char> &img,
            unsigned int width, unsigned int height,
            const float inv_mvp[16],
            const Options &opts
    );

    void reset();
private:
    std::vector<float> m_img;
    unsigned int m_spp_total = 0;
    unsigned int m_spp_max = 100;

    void accumulate_and_write(
            std::vector<unsigned char> &img,
            unsigned int pix_id,
            float L, int spp
    );
};

} // namespace rtnpr