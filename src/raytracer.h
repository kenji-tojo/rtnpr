#pragma once

#include <memory>
#include <vector>

#include "options.hpp"
#include "scene.hpp"
#include "sampler.hpp"
#include "camera.hpp"

namespace rtnpr {

class RayTracer {
public:
    Scene scene;

    void step(
            std::vector<unsigned char> &img,
            unsigned int width, unsigned int height,
            const Camera &camera,
            const Options &opts
    );

    void reset();
private:
    std::vector<Eigen::Vector3f> m_img;
    std::vector<float> m_alpha_obj;
    std::vector<float> m_alpha_line;

    unsigned int m_spp = 0;

    void accumulate_and_write(
            std::vector<unsigned char> &img,
            unsigned int pix_id,
            Eigen::Vector3f &L,
            float alpha_obj, float alpha_line,
            const Options &opts
    );
};

} // namespace rtnpr