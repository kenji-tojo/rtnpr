#pragma once

#include <memory>

#include "rtnpr_math.hpp"
#include "object.hpp"

namespace rtnpr {

class Plane: public Object {
public:
    int check_res = 10;
    bool checkerboard = true;

    static std::shared_ptr<Plane> create()
    {
        auto plane = std::make_shared<Plane>();
        plane->transform->scale = 5.f;
        plane->apply_transform();
        plane->mat_id = 0;
        return plane;
    }

    Plane();
    ~Plane();

    void ray_cast(const Ray &ray, Hit &hit) const override;
    void apply_transform() override;

private:
    Eigen::Vector3f m_center = Eigen::Vector3f::Zero();
    Eigen::Vector3f m_normal = Eigen::Vector3f::UnitZ();
    Eigen::Vector3f m_b1 = Eigen::Vector3f::UnitX();
    Eigen::Vector3f m_b2 = Eigen::Vector3f::UnitY();

    float m_width = 1.f;
    float m_height = 1.f;

};

} // namespace rtnpr
