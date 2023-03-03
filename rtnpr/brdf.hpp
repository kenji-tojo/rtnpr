#pragma once

#include "rtnpr_math.hpp"
#include "sampler.hpp"

namespace rtnpr {

class BRDF {
public:
    float albedo = .5f;
    bool reflect_line = false;

    explicit BRDF(float _albedo = .5f) : albedo(_albedo) {}

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
            float &pdf_val,
            Sampler<float> &sampler
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
        pdf_val  = pdf(nrm, wo, wi);
    }

private:
};

class SpecularBRDF: public BRDF {
public:
    SpecularBRDF() {
        this->reflect_line = true;
        this->albedo = .5f;
    }

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
            float &pdf_val,
            Sampler<float> &sampler
    ) const override {
        wi = -wo + 2.f * wo.dot(nrm) * nrm;
        brdf_val = math::max(0.f, wi.dot(nrm)) * this->albedo;
        pdf_val  = pdf(nrm, wo, wi);
    }

private:
};

class GlossyBRDF: public BRDF {
public:
    int power = 5000;

    GlossyBRDF() {
        this->reflect_line = true;
        this->albedo = .5f;
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
            float &pdf_val,
            Sampler<float> &sampler
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
        pdf_val  = pdf(nrm, wo, wi);
    }

private:
};

class PhongBRDF: public BRDF {
public:
    BRDF diffuse;
    GlossyBRDF glossy;

    float albedo = .7f;

    bool reflect_line = false;
    float kd = .2f;

    explicit PhongBRDF(float _albedo = .5f) : albedo(_albedo)
    {
        diffuse.albedo = 1.f;
        glossy.albedo = 1.f;
        glossy.power = 5;
    }

    [[nodiscard]] float eval(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        float kd_ = math::clip(kd, 0.f, 1.f);
        return albedo * (kd_ * diffuse.eval(nrm, wo, wi) + (1.f-kd_) * glossy.eval(nrm, wo, wi));
    }

    [[nodiscard]] float pdf(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            const Eigen::Vector3f &wi
    ) const override {
        float kd_ = math::clip(kd, 0.f, 1.f);
        return kd_ * diffuse.pdf(nrm, wo, wi) + (1.f-kd_) * glossy.pdf(nrm, wo, wi);
    }

    void sample_dir(
            const Eigen::Vector3f &nrm,
            const Eigen::Vector3f &wo,
            Eigen::Vector3f &wi,
            float &brdf_val,
            float &pdf_val,
            Sampler<float> &sampler
    ) const override {
        float kd_ = math::clip(kd, 0.f, 1.f);
        if (sampler.sample() < kd_) {
            diffuse.sample_dir(nrm, wo, wi, brdf_val, pdf_val, sampler);
            pdf_val *= kd_;
        }
        else {
            glossy.sample_dir(nrm, wo, wi, brdf_val, pdf_val, sampler);
            pdf_val *= (1.f-kd_);
        }
        brdf_val = eval(nrm, wo, wi);
    }

private:
};

} // namespace rtnpr