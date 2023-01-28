#pragma once

#include <random>

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

} // namespace rtnpr