#pragma once

#include <vector>

#include "options.hpp"

#include "delfem2/mat4.h"

namespace rtnpr {

class RayTracer {
public:
    void step(
            std::vector<unsigned char> &img,
            unsigned int width, unsigned int height,
            const delfem2::CMat4<float> &mvp,
            const Options &opts
    );

    void reset();

private:

};

} // namespace rtnpr