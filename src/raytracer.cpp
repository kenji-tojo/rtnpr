#include "raytracer.h"

#include <thread>

#include "delfem2/thread.h"

#include "rtnpr_math.hpp"
#include "linetest.hpp"


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

    unsigned int nthreads = std::thread::hardware_concurrency();
    std::vector<UniformPixelSampler<float>> sampler_pool(nthreads);
    auto func0 = [&](int ih, int iw, int tid) {
        const auto n_path = opts.rt.n_path;
        if (n_path <= 0) { return; }
        if (m_spp_total + n_path > m_spp_max) { return; }

        const unsigned int pix_id = ih * width + iw;

        float L = 0.f;
        float weight = 1.f / float(n_path);
        const auto light_dir = Vector3f(1,1,1).normalized();

        const float bd = .2f;
        for (int ii = 0; ii < n_path; ++ii)
        {
            const auto [cen_w,cen_h] = sampler_pool[tid].sample(
                    (float(iw)+.5f)/float(width),
                    (float(ih)+.5f)/float(height),
                    (1.f+bd)/float(width), (1.f+bd)/float(height)
            );
            Hit hit;
            Ray ray(inv_mvp, cen_w, cen_h);
            scene.ray_cast(ray, hit);
            if (hit.obj_id >= 0) {
                float c = (hit.nrm.dot(light_dir)+1.f)*.5f;
                c = c*.8f+.1f;
                L += weight * c;
            }
        }

        float t = float(m_spp_total) / float(m_spp_total + n_path);
        for (int ii = 0; ii < 3; ++ii) {
            auto kk = 3*pix_id+ii;
            m_img[kk] = t * m_img[kk] + (1.f-t) * L;
            img[kk] = math::to_u8(m_img[kk]);
        }
    };
    delfem2::parallel_for(width, height, func0, nthreads);

    m_spp_total += opts.rt.n_path;
}

void RayTracer::reset()
{
    auto size = m_img.size();
    m_img.clear();
    m_img.resize(size,0.);
    m_spp_total = 0;
}

} // namespace rtnpr