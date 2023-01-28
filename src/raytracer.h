#pragma once

#include <memory>
#include <vector>

#include "delfem2/mat4.h"

#include "options.hpp"
#include "scene.hpp"

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
    std::vector<double> m_img;
    unsigned int m_spp_total = 0;
    unsigned int m_spp_max = 100000;

};

void load_bunny(
        std::vector<double> &xyz_vec,
        std::vector<unsigned int> &tri_vec
);

} // namespace rtnpr