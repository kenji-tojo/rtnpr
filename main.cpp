#include <memory>

#include "viewer.h"
#include "trimesh.h"

#include <Eigen/Geometry>
#include <igl/readOBJ.h>

int main()
{
    using namespace rtnpr;
    using namespace Eigen;

    MatrixXf V;
    MatrixXi F;
    igl::readOBJ("assets/bunny_2k.obj",V,F);
    auto mesh = std::make_shared<TriMesh>(std::move(V),std::move(F));
    {
        mesh->transform->scale = .05f;
        mesh->transform->angle_axis[0] = -.5*M_PI;
        mesh->transform->shift[1] = -1.;
        mesh->apply_transform();
    }

    Scene scene;
    scene.add(mesh);

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 512;
    viewer.tex_height = 512;
#endif

    viewer.set_scene(scene);
    viewer.open();
}