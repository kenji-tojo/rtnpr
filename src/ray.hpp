#pragma once

#include <limits>

#include <Eigen/Dense>

namespace rtnpr {

struct Ray{
public:
    template<typename VEC3>
    Ray(const VEC3 &_org, const VEC3 &_dir)
    {
        using namespace Eigen;
        org = Vector3f(_org[0],_org[1],_org[2]);
        dir = Vector3f(_dir[0],_dir[1],_dir[2]);
    }

    template<typename T>
    Ray(const T mMVPd_inv[16], T w, T h)
    {
        using namespace Eigen;
        const T ps[4] = {w*2.f-1.f, h*2.f-1.f, +1.f, 1.f};
        const T pe[4] = {w*2.f-1.f, h*2.f-1.f, -1.f, 1.f};
        org = Vec3_Mat4Vec3_Homography<Vector3f>(mMVPd_inv,ps);
        const auto tar = Vec3_Mat4Vec3_Homography<Vector3f>(mMVPd_inv,pe);
        dir = (tar-org).normalized();
    }

    Eigen::Vector3f org;
    Eigen::Vector3f dir;
    float tmin = 0.f;
    float tmax = std::numeric_limits<float>::max();

private:
    template<typename T>
    static inline void MatVec4(
            T v[4],
            const T A[16],
            const T x[4]
    ) {
        v[0] = A[0 * 4 + 0] * x[0] + A[0 * 4 + 1] * x[1] + A[0 * 4 + 2] * x[2] + A[0 * 4 + 3] * x[3];
        v[1] = A[1 * 4 + 0] * x[0] + A[1 * 4 + 1] * x[1] + A[1 * 4 + 2] * x[2] + A[1 * 4 + 3] * x[3];
        v[2] = A[2 * 4 + 0] * x[0] + A[2 * 4 + 1] * x[1] + A[2 * 4 + 2] * x[2] + A[2 * 4 + 3] * x[3];
        v[3] = A[3 * 4 + 0] * x[0] + A[3 * 4 + 1] * x[1] + A[3 * 4 + 2] * x[2] + A[3 * 4 + 3] * x[3];
    }

    template<typename VEC3, typename T1, typename T2>
    static inline VEC3 Vec3_Mat4Vec3_Homography(
            const T1 a[16],
            const T2 x0[3]
    ) {
        const T1 x1[4] = {
                (T1) x0[0],
                (T1) x0[1],
                (T1) x0[2],
                1};
        T1 y1[4];
        MatVec4(y1, a, x1);
        return VEC3{
                y1[0] / y1[3],
                y1[1] / y1[3],
                y1[2] / y1[3]
        };
    }
};

} // rtnpr