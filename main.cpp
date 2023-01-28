#include <memory>

#include "viewer.h"
#include "trimesh.h"
#include "rtnpr_math.hpp"

#include <Eigen/Geometry>
#include <igl/readOBJ.h>

int main()
{
    using namespace rtnpr;
    using namespace Eigen;

    MatrixXd V;
    MatrixXi F;
    igl::readOBJ("assets/bunny_2k.obj",V,F);
    auto mesh = std::make_shared<TriMesh>(std::move(V),std::move(F));
    {
        double scale = 0.05;
        Matrix3d rot;
        rot = AngleAxisd(-.5*M_PI, Vector3d::UnitX())
              * AngleAxisd(0.,  Vector3d::UnitY())
              * AngleAxisd(0., Vector3d::UnitZ());
        const auto shift = -1*Vector3d::UnitY();
        mesh->transform(scale, rot, shift);
    }

    Scene scene;
    scene.add(mesh);

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 800;
    viewer.tex_height = 800;
#endif

    viewer.set_scene(scene);
    viewer.open();
}