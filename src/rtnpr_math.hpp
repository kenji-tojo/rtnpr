#pragma once

#include <cstdint>
#include <cmath>

#include <Eigen/Dense>

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif

namespace rtnpr::math {

template<typename Scalar>
Scalar max(Scalar a, Scalar b)
{
    return a < b ? b : a;
}

template<typename Scalar>
Scalar min(Scalar a, Scalar b)
{
    return a < b ? a : b;
}

template<typename Scalar>
Scalar clip(Scalar a, Scalar lo, Scalar hi)
{
    return max(lo, min(hi, a));
}

template<typename Scalar>
uint8_t to_u8(Scalar a)
{
    return uint8_t(Scalar(255) * clip(a, Scalar(0), Scalar(1)));
}

} // namespace rtnpr::math