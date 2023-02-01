#include <memory>

#include "viewer.h"
#include "trimesh.h"
#include "plane.h"

#include <Eigen/Geometry>
#include <igl/readOBJ.h>

int main()
{
    using namespace rtnpr;
    using namespace Eigen;

    MatrixXf V;
    MatrixXi F;
    igl::readOBJ("assets/bunny_309_faces.obj",V,F);
    auto mesh = TriMesh::create(V,F);
    {
        float scale = .03f;
        mesh->transform->scale = scale;
//        mesh->transform->shift = -scale*V.row(0);
        mesh->apply_transform();
    }

    auto plane = Plane::create();
    {
        plane->transform->scale = 5.f;
        plane->apply_transform();
        plane->mat_id = 1;
    }

    Scene scene;
    scene.add(mesh);
    scene.add(plane);

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 800;
    viewer.tex_height = 800;
#endif

    viewer.set_scene(scene);
    viewer.open();
}