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

template<typename VEC3>
void create_local_frame(const VEC3 &nrm, VEC3 &b1, VEC3 &b2)
{
    const double sign = nrm.z() >= 0 ? 1 : -1;
    const double a = -1.0 / (sign + nrm.z());
    const double b = nrm.x() * nrm.y() * a;
    b1 = VEC3(1.0 + sign * nrm.x() * nrm.x() * a, sign * b, -sign * nrm.x());
    b2 = VEC3(b, sign + nrm.y() * nrm.y() * a, -nrm.y());

#define EPS 1e-8
    assert(std::abs(b1.norm() - 1.) < EPS);
    assert(std::abs(b2.norm() - 1.) < EPS);
    assert(std::abs(b1.dot(nrm)) < EPS);
    assert(std::abs(b2.dot(nrm)) < EPS);
    assert(std::abs(b1.dot(b2)) < EPS);
#undef EPS
}

} // namespace rtnpr::math