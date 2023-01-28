#include <memory>
#include <vector>
#include <iostream>

#include "viewer.h"
#include "trimesh.h"

int main()
{
    using namespace rtnpr;
    namespace dfm2 = delfem2;

    std::vector<double> xyz_vec;
    std::vector<unsigned int> tri_vec;
    load_bunny(xyz_vec,tri_vec);

    Scene scene;
    auto mesh = std::make_shared<TriMesh>(xyz_vec,tri_vec);
    scene.add(std::move(mesh));

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 800;
    viewer.tex_height = 800;
#endif

    viewer.set_scene(scene);
    viewer.open();
}