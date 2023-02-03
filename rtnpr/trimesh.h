#pragma once

#include <memory>
#include <vector>

#include "object.hpp"

namespace rtnpr {

class TriMesh: public Object {
public:
    static std::shared_ptr<TriMesh> create(
            const Eigen::MatrixXf &V, const Eigen::MatrixXi &F
    ) {
        return std::make_shared<TriMesh>(V,F);
    }

    TriMesh(Eigen::MatrixXf V, Eigen::MatrixXi F);
    ~TriMesh();

    void ray_cast(const Ray &ray, Hit &hit) const override;
    void apply_transform() override;

private:
    class BVH;
    std::unique_ptr<BVH> m_bvh;

    Eigen::MatrixXf m_refV;
    Eigen::MatrixXi m_F;

};

} // namespace rtnpr