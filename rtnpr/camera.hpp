#pragma once

#include <memory>

#include "rtnpr_math.hpp"
#include "ray.hpp"

namespace rtnpr {

class Camera {
public:
    static std::shared_ptr<Camera> create() { return std::make_shared<Camera>(); }

    float radius = 5.f;
    float phi = float(M_PI)*1.5f;
    float z = .5f;
    float fov_rad = float(M_PI)/12.f;

    void shift_z(float disp)
    {
        disp *= -1.f;
        z = math::clip(z+disp,-.99f,.99f);
    }

    void shift_phi(float disp)
    {
        disp *= -1.f;
        float tpi = 2.f * float(M_PI);
        phi += disp * tpi;
        while (phi > tpi) { phi -= tpi; }
        while (phi < 0.f) { phi += tpi; }
    }

    void shift_radius(float disp)
    {
        disp *= -1.f;
        radius = math::max(.01f, radius+disp);
    }

    [[nodiscard]] Ray spawn_ray(float w, float h) const
    {
        using namespace Eigen;
        const Vector3f eye = pos();
        const Vector3f to = (m_tar - eye).normalized();
        const Vector3f right = to.cross(m_up).normalized();
        const Vector3f up = right.cross(to).normalized();

        float ic = 1.f / std::cos(.5f*fov_rad);
        float screen_x = ic * (math::clip(w,0.f,1.f)*2.f-1.f);
        float screen_y = ic * (math::clip(h,0.f,1.f)*2.f-1.f);

        Vector3f dir;
        dir = to + screen_x*right + screen_y*up;
        dir.normalize();

        return {eye, dir};
    }

private:

    [[nodiscard]] Eigen::Vector3f pos() const
    {
        using namespace std;
        using namespace Eigen;
        float sxy = sqrt(math::max(0.f,1.f-z*z));
        Vector3f p{sxy*cos(phi), sxy*sin(phi), z};
        return radius * p;
    }

    const Eigen::Vector3f m_up = Eigen::Vector3f::UnitZ();
    const Eigen::Vector3f m_tar = Eigen::Vector3f::Zero();

};

} // namespace rtnpr