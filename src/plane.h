#pragma once

#include <memory>

#include "rtnpr_math.hpp"
#include "object.hpp"

namespace rtnpr {

class Plane: public Object {
public:
    int check_res = 10;

    static std::shared_ptr<Plane> create()
    {
        return std::make_shared<Plane>();
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
