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
            const delfem2::CMat4<float> &mvp,
            const Options &opts
    );

    void reset();

private:

};

void load_bunny(
        std::vector<double> &xyz_vec,
        std::vector<unsigned int> &tri_vec
);

} // namespace rtnpr