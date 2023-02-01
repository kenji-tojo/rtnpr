#pragma once

#include "rtnpr_math.hpp"
#include "sampler.hpp"

namespace rtnpr {

class BRDF {
public:
    float albedo = .7f;
    bool reflect_line = false;

    [[nodiscard]] virtual float eval(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const {
        return albedo * math::max(0.f, nrm.dot(wi)) / float(M_PI);
    }

    [[nodiscard]] virtual float pdf(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const {
        // cosine-weighted
        return std::max(0.f,nrm.dot(wi)) / float(M_PI);
    }

    virtual void sample_dir(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            Eigen::Vector3f &wi,
            float &brdf_val,
            UniformSampler<float> &sampler
    ) const {
        using namespace std;
        using namespace Eigen;

        Vector3f b1, b2;
        math::create_local_frame(nrm, b1, b2);

        float z = sqrt(math::max(0.f,sampler.sample()));
        float rxy = sqrt(math::max(0.f,1.f-z*z));
        float phi = 2.f*float(M_PI)*sampler.sample();

        wi = z*nrm + rxy*cos(phi)*b1 + rxy*sin(phi)*b2;
        brdf_val = eval(nrm, wo, wi);
    }

private:
};

class SpecularBRDF: public BRDF {
public:
    SpecularBRDF() {this->reflect_line = true;}

    [[nodiscard]] float eval(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        return 0.f;
    }

    [[nodiscard]] float pdf(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        return 1.f;
    }

    void sample_dir(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            Eigen::Vector3f &wi,
            float &brdf_val,
            UniformSampler<float> &sampler
    ) const override {
        wi = -wo + 2.f * wo.dot(nrm) * nrm;
        brdf_val = math::max(0.f, wi.dot(nrm)) * this->albedo;
    }

private:
};

class GlossyBRDF: public BRDF {
public:
    int power = 200;

    GlossyBRDF() {
        this->reflect_line = true;
        this->albedo = .9f;
    }

    [[nodiscard]] float eval(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        using namespace std;
        using namespace Eigen;
        Vector3f r = -wo + 2.f * nrm.dot(wo) * nrm;
        float c = math::max(0.f, r.dot(wi));
        float w = this->albedo * .5f * float(power+2) * pow(c, float(power)) / float(M_PI);
        return w * math::max(0.f, nrm.dot(wi));
    }

    [[nodiscard]] float pdf(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        using namespace std;
        using namespace Eigen;
        Vector3f r = -wo + 2.f * nrm.dot(wo) * nrm;
        float c = math::max(0.f, r.dot(wi));
        return .5f * float(power+1) * pow(c,float(power)) / float(M_PI);
    }

    void sample_dir(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            Eigen::Vector3f &wi,
            float &brdf_val,
            UniformSampler<float> &sampler
    ) const override {
        using namespace std;
        using namespace Eigen;

        Vector3f r = -wo + 2.f * nrm.dot(wo) * nrm;
        Vector3f b1, b2;
        math::create_local_frame(r, b1, b2);

        float z = pow(math::max(0.f,sampler.sample()), 1.f/float(power+1));
        float rxy = sqrt(math::max(0.f,1.f-z*z));
        float phi = 2.f*float(M_PI)*sampler.sample();

        wi = z*r + rxy*cos(phi)*b1 + rxy*sin(phi)*b2;
        brdf_val = eval(nrm, wo, wi);
    }

private:
};

} // namespace rtnpr