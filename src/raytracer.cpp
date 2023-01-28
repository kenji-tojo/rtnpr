#include "raytracer.h"

#include "delfem2/srch_trimesh3_class.h"
#include "delfem2/msh_normal.h"
#include "delfem2/msh_io_ply.h"

#include "rtnpr_math.hpp"


namespace dfm2 = delfem2;

namespace rtnpr {

void RayTracer::step(
        std::vector<unsigned char> &img,
        unsigned int width, unsigned int height,
        const delfem2::CMat4<float> &mvp,
        const Options &opts
) {
    using namespace std;
    using namespace Eigen;

    if (img.size() != width * height * 3) {
        img.clear();
        img.resize(height*width*3,0);
    }

    const dfm2::CMat4d inv_mvp = delfem2::Inverse_Mat4(mvp.data());
    auto func0 = [&](int ih, int iw) {
        const auto [org, dir] = delfem2::RayFromInverseMvpMatrix(
                inv_mvp.data(), iw, ih, width, height);
        Hit hit;
        Ray ray(
                Vector3f((float)org[0], (float)org[1], (float)org[2]),
                Vector3f((float)dir[0], (float)dir[1], (float)dir[2])
        );

        const auto light_dir = Vector3f(1,1,1).normalized();

        scene.ray_cast(ray, hit);
        if (hit.obj_id >= 0) {
            float c = (hit.nrm.dot(light_dir)+1.f)*.5f;
            c = c*.8f+.1f;
            for (int jj = 0; jj < 3; ++jj) {
                img[3 * (ih * width + iw) + jj] = math::to_u8(c);
            }
        }
        else {
            float c = 0.f;
            for (int jj = 0; jj < 3; ++jj) {
                img[3 * (ih * width + iw) + jj] = math::to_u8(c);
            }
        }
    };
    delfem2::parallel_for(width, height, func0);
}

void RayTracer::reset()
{

}

void load_bunny(
        std::vector<double> &xyz_vec,
        std::vector<unsigned int> &tri_vec
) {
    dfm2::Read_Ply(xyz_vec, tri_vec, "./assets/bunny_2k.ply");
    std::cout << "tri count: " << tri_vec.size()/3 << std::endl;
    dfm2::Rotate_Points3(xyz_vec, -M_PI*0.5, 0.0, 0.0);
    dfm2::Normalize_Points3(xyz_vec, 2.5);
}

} // namespace rtnpr