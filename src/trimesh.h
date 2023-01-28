#pragma once

#include <memory>
#include <vector>

#include "object.hpp"

namespace rtnpr {

class TriMesh: public Object {
public:
    TriMesh(Eigen::MatrixXd &&V, Eigen::MatrixXi &&F);
    ~TriMesh();

    void ray_cast(const Ray &ray, Hit &hit) const override;
    void transform(
            double scale,
            const Eigen::Matrix3d &rot,
            const Eigen::Vector3d &shift
    ) override;

private:
    class BVH;
    std::unique_ptr<BVH> m_bvh;

    Eigen::MatrixXd m_refV;
    Eigen::MatrixXi m_F;

};

} // namespace rtnpr