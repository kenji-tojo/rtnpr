#pragma once

#include "rtnpr_math.hpp"
#include "sampler.hpp"

namespace rtnpr {

class BRDF {
public:
    float albedo = .8f;

    virtual float eval(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) {
        return albedo * math::max(0.f, nrm.dot(wi)) / float(M_PI);
    }

    virtual void sample_dir(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            Eigen::Vector3f &wi,
            float &pdf,
            float &brdf_val,
            UniformSampler<float> &sampler
    ) {
        using namespace std;
        using namespace Eigen;

        Vector3f b1, b2;
        math::create_local_frame(nrm, b1, b2);

        float z = sqrt(math::max(0.f,sampler.sample()));
        float sxy = sqrt(math::max(0.f,1.f-z*z));
        float phi = 2.f*float(M_PI)*sampler.sample();

        wi = z*nrm + sxy*cos(phi)*b1 + sxy*sin(phi)*b2;
        pdf = std::max(0.f,nrm.dot(wi)) / float(M_PI);
        brdf_val = eval(nrm, wo, wi);
    }

private:
};

} // namespace rtnpr