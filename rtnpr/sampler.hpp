#pragma once

#include <tuple>
#include <random>

#include "rtnpr_math.hpp"

namespace rtnpr {

template<typename T>
class Sampler {
public:
    Sampler() : rd(), gen(rd()), dis(T(0), T(1))
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
std::pair<T,T> sample_disc(Sampler<T> &sampler)
{
    using namespace std;
    T theta = sampler.sample() * 2. * M_PI;
    T r = sqrt(sampler.sample());
    return make_pair(r*cos(theta), r*sin(theta));
}

template<typename T>
std::pair<T,T> sample_pixel(
        T cen_w, T cen_h,
        T diam_w, T diam_h,
        Sampler<T> &sampler
) {
    T d_w = diam_w * sampler.sample() - T(.5) * diam_w;
    T d_h = diam_h * sampler.sample() - T(.5) * diam_h;
    return std::make_pair(cen_w+d_w, cen_h+d_h);
}

} // namespace rtnpr