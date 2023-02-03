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

template<typename Vector3, typename Scalar>
void clip3(Vector3 &v, Scalar lo, Scalar hi)
{
    v[0] = clip(v[0], lo, hi);
    v[1] = clip(v[1], lo, hi);
    v[2] = clip(v[2], lo, hi);
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

#define EPS 1e-5
    assert(std::abs(b1.norm() - 1.) < EPS);
    assert(std::abs(b2.norm() - 1.) < EPS);
    assert(std::abs(b1.dot(nrm)) < EPS);
    assert(std::abs(b2.dot(nrm)) < EPS);
    assert(std::abs(b1.dot(b2)) < EPS);
#undef EPS
}

template<typename Float>
inline Float tone_map_Reinhard(const Float c, const Float burn)
{
    static_assert(std::is_floating_point_v<Float>);
    return c * (1.0 + c / (burn * burn)) / (1 + c);
}

template<typename Vector3, typename Float>
inline void Reinhard3(Vector3 &v, const Float burn)
{
    v[0] = tone_map_Reinhard(v[0], burn);
    v[1] = tone_map_Reinhard(v[1], burn);
    v[2] = tone_map_Reinhard(v[2], burn);
}

template<typename Float>
inline Float sigmoid(const Float c)
{
    static_assert(std::is_floating_point_v<Float>);
    return Float(2.) * math::max<Float>(0., -.5+1./(1.+std::exp(-c)));
}

template<typename Vector3>
inline void sigmoid3(Vector3 &v)
{
    v[0] = sigmoid(v[0]);
    v[1] = sigmoid(v[1]);
    v[2] = sigmoid(v[2]);
}

template<typename Vector3>
inline void floor3(Vector3 &v)
{
    v[0] = std::floor(v[0]);
    v[1] = std::floor(v[1]);
    v[2] = std::floor(v[2]);
}

} // namespace rtnpr::math