#include <iostream>
#include <memory>

#include "viewer/viewer.h"
#include "rtnpr/trimesh.h"

#include <Eigen/Geometry>

#if defined (RTNPR_TEST)
#include <igl/readOBJ.h>
#endif

#if defined(RTNPR_NANOBIND)
#include <nanobind/nanobind.h>
#include <nanobind/tensor.h>
#endif


using namespace rtnpr;

namespace {

Image<float, PixelFormat::RGBA> run_gui(Eigen::MatrixXf &V, Eigen::MatrixXi &F)
{
    using namespace viewer;
    using namespace Eigen;
    using namespace std;

    cout << "run gui: input mesh with " << V.rows()
         << " vertices and "
         << F.rows() << " faces " << endl;

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

    Image<float, PixelFormat::RGBA> img;
    viewer.set_scene(scene);
    viewer.open(img);
    if (img.pixels() > 0) {
        cout << "return screenshot with "
             << img.width()
             << "x" << img.height()
             << "x" << img.channels()
             <<  " pixels" << endl;
    }
    return img;
}

} // namespace


#if defined(RTNPR_TEST)
int main()
{
    using namespace Eigen;
    MatrixXf V;
    MatrixXi F;
    igl::readOBJ("assets/bunny.obj",V,F);
    run_gui(V,F);
}
#endif


#if defined(RTNPR_NANOBIND)
namespace nb = nanobind;

using namespace nb::literals;

NB_MODULE(rtnpr, m) {
    m.def("run_gui", [](
            nb::tensor<float, nb::shape<nb::any, 3>> &V_tensor,
            nb::tensor<int, nb::shape<nb::any, 3>> &F_tensor
    ) {
        using namespace std;
        using namespace Eigen;

        MatrixXf V;
        MatrixXi F;

        V.resize(V_tensor.shape(0),3);
        for (int ii = 0; ii < V.rows(); ++ii) {
            V(ii,0) = V_tensor(ii,0);
            V(ii,1) = V_tensor(ii,1);
            V(ii,2) = V_tensor(ii,2);
        }

        F.resize(F_tensor.shape(0),3);
        for (int kk = 0; kk < F.rows(); ++kk) {
            F(kk,0) = F_tensor(kk,0);
            F(kk,1) = F_tensor(kk,1);
            F(kk,2) = F_tensor(kk,2);
        }

        auto img = run_gui(V,F);
        size_t shape[3]{img.width(), img.height(), img.channels()};
        return nb::tensor<nb::numpy, float>{img.data(),3, shape};
    });
}
#endif // #if defined(RTNPR_NANOBIND)
