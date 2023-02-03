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
    std::vector<Eigen::Vector3f> m_foreground;
    std::vector<float> m_alpha_fore;
    std::vector<float> m_alpha_line;

    unsigned int m_spp = 0;

    void accumulate_sample(
            std::vector<unsigned char> &img,
            unsigned int pix_id,
            Eigen::Vector3f &L,
            float alpha_fore, float alpha_line,
            const Options &opts
    );

    template<typename Scalar>
    void composite(
            std::vector<Scalar> &img,
            unsigned int pix_id,
            const Options &opts
    );
};

} // namespace rtnpr