#include <iostream>
#include <memory>
#include <string>

#include "viewer/viewer.h"
#include "rtnpr/trimesh.h"


#include <igl/readOBJ.h>


using namespace rtnpr;


int main()
{
    using namespace Eigen;
    using namespace std;

    auto scene = make_shared<Scene>();
    {
        MatrixXf V;
        MatrixXi F;
        igl::readOBJ("assets/bunny.obj",V,F);
        scene->add(std::make_shared<TriMesh>(V,F));
    }

    scene->camera->position = Vector3f(0.f,-180.f,100.f);

    viewer::Viewer viewer;
    viewer.set_scene(scene);
    viewer.open();
}
