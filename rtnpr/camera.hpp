#pragma once

#include <memory>

#include "rtnpr_math.hpp"
#include "ray.hpp"

namespace rtnpr {

class Camera {
public:
    static std::shared_ptr<Camera> create() { return std::make_shared<Camera>(); }

    Eigen::Vector3f position = Eigen::Vector3f(1.f,1.f,1.f);
    float fov_rad = float(M_PI)/12.f;

    void look_at(const Eigen::Vector3f &target) { m_to = (target-position).normalized(); }

    [[nodiscard]] Ray spawn_ray(float w, float h) const
    {
        using namespace Eigen;
        const Vector3f right = m_to.cross(m_up).normalized();
        const Vector3f up = right.cross(m_to).normalized();

        float ic = 1.f / std::cos(.5f*fov_rad);
        float screen_x = ic * (math::clip(w,0.f,1.f)*2.f-1.f);
        float screen_y = ic * (math::clip(h,0.f,1.f)*2.f-1.f);

        Vector3f dir;
        dir = m_to + screen_x*right + screen_y*up;
        dir.normalize();

        return {position, dir};
    }

private:
    Eigen::Vector3f m_up = Eigen::Vector3f::UnitZ();
    Eigen::Vector3f m_to = -Eigen::Vector3f::UnitX();

};

} // namespace rtnpr