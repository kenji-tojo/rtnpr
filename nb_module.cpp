#include <iostream>
#include <memory>
#include <string>

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
             << " pixels" << endl;
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


namespace {

bool py_stob(const std::string &str)
{
    if (str == "True") { return true; }
    else if (str == "False") { return false; }
    else {
        std::cerr << "py_stob error: str is not [True | False]" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

std::string fmt_(bool val) { return val ? "true" : "false"; }
template<typename T> T fmt_(T val) { static_assert(!std::is_same_v<T,bool>); return val; }

void import_options(const nb::dict &src_dict, rtnpr::Options &opts)
{
    using namespace std;

#define ASSIGN_FIELD(field, cast_fn)                       \
if (key == #field) {                                       \
    opts.field = cast_fn(value);                           \
    cout << #field << " = " << ::fmt_(opts.field) << endl; \
}

    cout << "--- importing options ---" << endl;
    for (const auto &item: src_dict)
    {
        auto key = string(nb::str(item.first).c_str());
        auto value = string(nb::str(item.second).c_str());
        ASSIGN_FIELD(rt.spp_frame, stoi)
        ASSIGN_FIELD(rt.spp, stoi)
        ASSIGN_FIELD(rt.depth, stoi)

        ASSIGN_FIELD(flr.linewidth, stof)
        ASSIGN_FIELD(flr.enable, py_stob)
        ASSIGN_FIELD(flr.line_only, py_stob)
        ASSIGN_FIELD(flr.wireframe, py_stob)
        ASSIGN_FIELD(flr.n_aux, stoi)

        ASSIGN_FIELD(tone.map_shading, py_stob)
    }
    cout << "---" << endl;

#undef ASSIGN_FIELD
}

void export_options(const rtnpr::Options &opts, nb::dict &dst_dict)
{
#define ASSIGN_FIELD(field) dst_dict[#field] = opts.field

    dst_dict = nb::dict();
    ASSIGN_FIELD(rt.spp_frame);
    ASSIGN_FIELD(rt.spp);
    ASSIGN_FIELD(rt.depth);

    ASSIGN_FIELD(flr.enable);
    ASSIGN_FIELD(flr.line_only);
    ASSIGN_FIELD(flr.normal);
    ASSIGN_FIELD(flr.position);
    ASSIGN_FIELD(flr.wireframe);
    ASSIGN_FIELD(flr.linewidth);
    ASSIGN_FIELD(flr.n_aux);

    ASSIGN_FIELD(tone.map_lines);
    ASSIGN_FIELD(tone.map_shading);

#undef ASSIGN_FIELD
}

} // namespace


NB_MODULE(rtnpr, m) {
    m.def("run_gui", [](
            nb::tensor<float, nb::shape<nb::any, 3>> &V_tensor,
            nb::tensor<int, nb::shape<nb::any, 3>> &F_tensor,
            const nb::dict &opts_dict
    ) {
        using namespace std;
        using namespace Eigen;

        auto opts = make_shared<rtnpr::Options>();
        auto camera = make_shared<rtnpr::Camera>();
        import_options(opts_dict,*opts);

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
        auto img_arr = nb::tensor<nb::numpy, float>{};
        if (img.pixels()>0) {
            size_t shape[3]{img.width(), img.height(), img.channels()};
            img_arr = nb::tensor<nb::numpy, float>{img.data(),3, shape};
        }

        nb::dict dict;
        export_options(*opts, dict);

        return nb::make_tuple(img_arr, dict);
    });
}
#endif // #if defined(RTNPR_NANOBIND)
