#include <memory>

#include "viewer/viewer.h"
#include "rtnpr/trimesh.h"

#include <Eigen/Geometry>
#include <igl/readOBJ.h>

#include <nanobind/nanobind.h>


namespace {

void run_gui()
{
    using namespace rtnpr;
    using namespace viewer;
    using namespace Eigen;

    MatrixXf V;
    MatrixXi F;
    igl::readOBJ("assets/bunny_309_faces.obj",V,F);
    auto mesh = TriMesh::create(V,F);
    {
        float scale = .03f;
        mesh->transform->scale = scale;
        mesh->apply_transform();
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

} // namespace


namespace nb = nanobind;

using namespace nb::literals;

NB_MODULE(rtnpr, m) {
    m.def("run_gui", []() { return run_gui(); });
}
