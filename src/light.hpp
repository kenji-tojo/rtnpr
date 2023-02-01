#pragma once

#include "rtnpr_math.hpp"
#include "sampler.hpp"

namespace rtnpr {

class Light {
public:
    float intensity = 2.f;
    Eigen::Vector3f color{1.f,1.f,1.f};
    int power = 200;

    void set_dir(Eigen::Vector3f _dir)
    {
        dir = std::move(_dir);
        dir.normalize();
    }

    [[nodiscard]] virtual Eigen::Vector3f Le(const Eigen::Vector3f &wi) const
    {
        using namespace Eigen;
        float c = math::max(0.f, wi.dot(dir));
        return float(power+2) * std::pow(c, float(power)) * intensity * color;
    }

    [[nodiscard]] virtual float pdf(const Eigen::Vector3f &wi) const
    {
        float c = math::max(0.f, wi.dot(dir));
        return .5f * float(power+1) * std::pow(c, float(power)) / float(M_PI);
    }

    virtual void sample_dir(
            Eigen::Vector3f &wi,
            UniformSampler<float> &sampler
    ) const {
        using namespace std;
        using namespace Eigen;

        Vector3f b1, b2;
        math::create_local_frame(dir, b1, b2);

        float z = pow(math::max(0.f,sampler.sample()), 1.f/float(power+1));
        float rxy = sqrt(math::max(0.f,1.f-z*z));
        float phi = 2.f*float(M_PI)*sampler.sample();

        wi = z*dir + rxy*cos(phi)*b1 + rxy*sin(phi)*b2;
    }

private:
    Eigen::Vector3f dir = Eigen::Vector3f(1.f,-1.f,3.f).normalized();

};

class DirectionalLight: public Light {
public:
    [[nodiscard]] Eigen::Vector3f Le(const Eigen::Vector3f &wi) const override
    {
        using namespace Eigen;
        return intensity * 5.f * color;
    }

    [[nodiscard]] float pdf(const Eigen::Vector3f &wi) const override
    {
        return 1.f;
    }

    void sample_dir(
            Eigen::Vector3f &wi,
            UniformSampler<float> &sampler
    ) const override {
        wi = dir;
    }

private:
    Eigen::Vector3f dir = Eigen::Vector3f::UnitZ();
};

} // rtnpr