#pragma once

#include <tuple>
#include <random>

#include "rtnpr_math.hpp"

namespace rtnpr {

template<typename T>
class UniformSampler {
public:
    UniformSampler() : rd(), gen(rd()), dis(T(0),T(1))
    {
        static_assert(std::is_floating_point_v<T>);
    }

    T sample() { return dis(gen); }
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<T> dis;
};

template<typename T>
class UniformDiscSampler {
public:
    UniformDiscSampler()
    {
        static_assert(std::is_floating_point_v<T>);
    }

    std::pair<T,T> sample()
    {
        using namespace std;
        T theta = sampler.sample() * 2. * M_PI;
        T r = sqrt(sampler.sample());
        return make_pair(r*cos(theta), r*sin(theta));
    }
private:
    UniformSampler<T> sampler;
};

} // namespace rtnpr