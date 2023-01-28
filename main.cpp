#include <memory>

#include "viewer.h"
#include "trimesh.h"

#include <igl/readOBJ.h>

int main()
{
    using namespace rtnpr;
    using namespace Eigen;

    double scale = 0.05;
    MatrixXd V;
    MatrixXi F;
    igl::readOBJ("assets/bunny_2k.obj",V,F);
    V *= scale;
    auto mesh = std::make_shared<TriMesh>(std::move(V),std::move(F));

    Scene scene;
    scene.add(std::move(mesh));

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 800;
    viewer.tex_height = 800;
#endif

    viewer.set_scene(scene);
    viewer.open();
}