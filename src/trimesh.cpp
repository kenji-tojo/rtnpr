#include "trimesh.h"

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>

#include <iostream>


namespace {

using Scalar  = float;
using Vec3    = bvh::v2::Vec<Scalar, 3>;
using BBox    = bvh::v2::BBox<Scalar, 3>;
using Tri     = bvh::v2::Tri<Scalar, 3>;
using Node    = bvh::v2::Node<Scalar, 3>;
using Bvh     = bvh::v2::Bvh<Node>;
using Ray     = bvh::v2::Ray<Scalar, 3>;

using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

} // namespace


namespace rtnpr {

class TriMesh::BVH {
public:
    BVH(const Eigen::MatrixXd &V, const Eigen::MatrixXi &F)
    {
        using namespace Eigen;

        std::vector<::Tri> tris(F.rows());
        for (int ii = 0; ii < F.rows(); ++ii) {
            const Vector3d &p0 = V.row(F(ii,0));
            const Vector3d &p1 = V.row(F(ii,1));
            const Vector3d &p2 = V.row(F(ii,2));
            tris[ii] = ::Tri(
                    ::Vec3(p0.x(),p0.y(),p0.z()),
                    ::Vec3(p1.x(),p1.y(),p1.z()),
                    ::Vec3(p2.x(),p2.y(),p2.z())
            );
        }

        bvh::v2::ThreadPool thread_pool;
        bvh::v2::ParallelExecutor executor(thread_pool);

        // Get triangle centers and bounding boxes (required for BVH builder)
        std::vector<::BBox> bboxes(tris.size());
        std::vector<::Vec3> centers(tris.size());
        executor.for_each(0, tris.size(), [&] (size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                bboxes[i]  = tris[i].get_bbox();
                centers[i] = tris[i].get_center();
            }
        });

        typename bvh::v2::DefaultBuilder<::Node>::Config config;
        config.quality = bvh::v2::DefaultBuilder<::Node>::Quality::High;
        m_bvh = std::make_unique<::Bvh>(bvh::v2::DefaultBuilder<::Node>::build(thread_pool, bboxes, centers, config));

        // This precomputes some data to speed up traversal further.
        m_precomputed_tris.resize(tris.size());
        executor.for_each(0, tris.size(), [&] (size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                auto j = m_should_permute ? m_bvh->prim_ids[i] : i;
                m_precomputed_tris[i] = tris[j];
            }
        });
    }

    bool ray_cast(const Ray &_ray, size_t &tri_id, float &dist, Eigen::Vector3f &nrm)
    {
        if (!m_bvh) { return  false; }
        auto &bvh = *m_bvh;

        auto ray = ::Ray {
                Vec3(_ray.org.x(), _ray.org.y(), _ray.org.z()), // Ray origin
                Vec3(_ray.dir.x(), _ray.dir.y(), _ray.dir.z()), // Ray direction
                _ray.tmin,               // Minimum intersection distance
                _ray.tmax              // Maximum intersection distance
        };

        static constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
        static constexpr size_t stack_size = 64;
        static constexpr bool use_robust_traversal = false;

        auto prim_id = invalid_id;
        Scalar u, v;

        // Traverse the BVH and get the u, v coordinates of the closest intersection.
        bvh::v2::SmallStack<::Bvh::Index, stack_size> stack;
        bvh.intersect<false, use_robust_traversal>(ray, bvh.get_root().index, stack,
                                                   [&] (size_t begin, size_t end) {
                                                       for (size_t i = begin; i < end; ++i) {
                                                           size_t j = m_should_permute ? i : bvh.prim_ids[i];
                                                           if (auto hit = m_precomputed_tris[j].intersect(ray)) {
                                                               prim_id = i;
                                                               std::tie(u, v) = *hit;
                                                           }
                                                       }
                                                       return prim_id != invalid_id;
                                                   });

        if (prim_id != invalid_id) {
            auto &n = m_precomputed_tris[prim_id].n;
            nrm = Eigen::Vector3f(n[0],n[1],n[2]).normalized();
            tri_id = prim_id;
            dist = ray.tmax;
//            std::cout
//                    << "Intersection found\n"
//                    << "  primitive: " << prim_id << "\n"
//                    << "  distance: " << ray.tmax << "\n"
//                    << "  barycentric coords.: " << u << ", " << v << std::endl;
            return true;
        } else {
//            std::cout << "No intersection found" << std::endl;
            return false;
        }
    }

private:
    std::unique_ptr<::Bvh> m_bvh;

    std::vector<::PrecomputedTri> m_precomputed_tris;

    // Permuting the primitive data allows to remove indirections during traversal, which makes it faster.
    const bool m_should_permute = true;
};

TriMesh::TriMesh(Eigen::MatrixXd &&V, Eigen::MatrixXi &&F)
        : m_V(std::move(V)), m_F(std::move(F))
{
    using namespace Eigen;
    m_bvh = std::make_unique<BVH>(m_V,m_F);
}

TriMesh::~TriMesh() = default;

void TriMesh::ray_cast(const Ray &ray, Hit &hit) const
{
    size_t tri_id;
    float dist;
    Eigen::Vector3f nrm;
    if (m_bvh->ray_cast(ray, tri_id, dist, nrm)) {
        if (dist >= hit.dist) { return; }
        hit.dist = dist;
        hit.tri_id = tri_id;
        hit.pos = ray.org + (dist-1e-8f) * ray.dir;
        hit.nrm = nrm;
        hit.obj_id = this->obj_id;
        hit.mat_id = this->mat_id;
    }
}

} // namespace rtnpr
