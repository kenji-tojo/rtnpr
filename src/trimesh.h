#pragma once

#include <memory>
#include <vector>

#include "object.hpp"

namespace rtnpr {

class TriMesh: public Object {
public:
    TriMesh(const std::vector<double> &xyz_vec, const std::vector<unsigned int> &tri_vec);
    ~TriMesh();

    void ray_cast(const Ray &ray, Hit &hit) const override;

private:
    class BVH;
    std::unique_ptr<BVH> m_bvh;

    Eigen::MatrixXd m_V;
    Eigen::MatrixXi m_F;

};

} // namespace rtnpr