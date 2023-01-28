#include "raytracer.h"

#include "rtnpr_math.hpp"

namespace rtnpr {

void RayTracer::step(
        std::vector<unsigned char> &img,
        unsigned int width, unsigned int height,
        const delfem2::CMat4<float> &mvp,
        const Options &opts
) {
    using namespace std;
    using namespace Eigen;

    if (img.size() != width * height * 3) { return; }

    unsigned int n_pix = width * height;
    for (int ii = 0; ii < n_pix; ++ii) {
        float c = .5f;
        for (int jj = 0; jj < 3; ++jj) {
            img[3*ii+jj] = math::to_u8(c);
        }
    }
}

void RayTracer::reset()
{

}

} // namespace rtnpr