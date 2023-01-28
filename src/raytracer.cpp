#include "raytracer.h"

#include "delfem2/thread.h"

#include "rtnpr_math.hpp"


namespace dfm2 = delfem2;

namespace rtnpr {

void RayTracer::step(
        std::vector<unsigned char> &img,
        unsigned int width, unsigned int height,
        const float inv_mvp[16],
        const Options &opts
) {
    using namespace std;
    using namespace Eigen;

    img.resize(height*width*3);
    if (m_img.size() != img.size()) {
        m_img.resize(img.size());
        reset();
    }

    auto func0 = [&](int ih, int iw) {
        Hit hit;
        Ray ray(
                inv_mvp,
                (float(iw)+.5f)/float(width),
                (float(ih)+.5f)/float(height)
        );

        const auto pix_id = ih * width + iw;
        const auto light_dir = Vector3f(1,1,1).normalized();

        scene.ray_cast(ray, hit);
        if (hit.obj_id >= 0) {
            float c = (hit.nrm.dot(light_dir)+1.f)*.5f;
            c = c*.8f+.1f;
            for (int jj = 0; jj < 3; ++jj) {
                m_img[3*pix_id+jj] = c;
            }
        }
        else {
            float c = 0.f;
            for (int jj = 0; jj < 3; ++jj) {
                m_img[3*pix_id+jj] = c;
            }
        }

        for (int jj = 0; jj < 3; ++jj) {
            auto kk = 3*pix_id+jj;
            img[kk] = math::to_u8(m_img[kk]);
        }
    };
    delfem2::parallel_for(width, height, func0);
}

void RayTracer::reset()
{
    auto size = m_img.size();
    m_img.clear();
    m_img.resize(size,0.);
    m_spp_total = 0;
}

} // namespace rtnpr