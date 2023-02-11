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

viewer::Command run_gui(
        Eigen::MatrixXf &&V,
        Eigen::MatrixXi &&F,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Options> opts
) {
    using namespace viewer;
    using namespace Eigen;
    using namespace std;

    cout << "run gui: input mesh with " << V.rows()
         << " vertices and "
         << F.rows() << " faces " << endl;

    auto mesh = TriMesh::create(V,F);

    auto scene = Scene::create();
    scene->plane().mat_id = 1;
    scene->add(mesh);

    Viewer viewer;
#if defined(NDEBUG)
    viewer.tex_width = 800;
    viewer.tex_height = 800;
#endif

    viewer.set_scene(std::move(scene));
    viewer.set_camera(std::move(camera));
    viewer.set_opts(std::move(opts));
    return viewer.open();
}


Image<float, PixelFormat::RGBA> run_headless(
        Eigen::MatrixXf &&V,
        Eigen::MatrixXi &&F,
        const Camera &camera,
        const Options &opts
) {
    using namespace viewer;
    using namespace Eigen;
    using namespace std;

    cout << "run headless: input mesh with " << V.rows()
         << " vertices and "
         << F.rows() << " faces " << endl;

    auto mesh = TriMesh::create(V,F);

    auto scene = Scene::create();
    scene->plane().mat_id = 1;
    scene->add(mesh);
    scene->light->look_at(Vector3f::Zero());

    const int spp = opts.rt.spp;
    const int spp_frame = opts.rt.spp_frame;

#if defined(NDEBUG)
    const int width = 800;
    const int height = 800;
#else
    const int width = 128;
    const int height = 128;
#endif

    RayTracer rt;
    for (int ii = 0; ii < spp/spp_frame; ++ii) {
        rt.step_headless(width, height, *scene, camera, opts);
        cout << "spp: " << rt.spp() << endl;
    }
    Image<float, PixelFormat::RGBA> img;
    rt.screenshot(img, opts);

    if (img.pixels() > 0) {
        cout << "returning image "
             << img.width()
             << "x" << img.height()
             << "x" << img.channels()
             << endl;
    }

    return img;
}

} // namespace


#if defined(RTNPR_TEST)
int main()
{
    using namespace Eigen;
    using namespace std;
    MatrixXf V;
    MatrixXi F;
    igl::readOBJ("assets/bunny.obj",V,F);
    auto camera = make_shared<rtnpr::Camera>();
    camera->position = Vector3f(0.f,-135.f,80.f);
    auto opts = make_shared<rtnpr::Options>();
    run_gui(std::move(V),std::move(F),camera,opts);
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

void import_params(
        const nb::dict &src_dict,
        rtnpr::Options &opts,
        rtnpr::Camera &camera
) {
    using namespace std;

#define ASSIGN_FIELD(trg, field, cast_fn)                 \
if (key == #field) {                                      \
    trg.field = cast_fn(value);                           \
    cout << #field << " = " << ::fmt_(trg.field) << endl; \
}

    cout << "--- importing options ---" << endl;
    for (const auto &item: src_dict)
    {
        auto key = string(nb::str(item.first).c_str());
        auto value = string(nb::str(item.second).c_str());

        auto pos = key.find(':');
        if (pos != string::npos) {
            cout << key.substr(0,pos) << ".";
            key = key.substr(pos+1,key.length());
        }

        ASSIGN_FIELD(opts, rt.spp_frame, stoi)
        ASSIGN_FIELD(opts, rt.spp, stoi)
        ASSIGN_FIELD(opts, rt.depth, stoi)

        ASSIGN_FIELD(opts, flr.linewidth, stof)
        ASSIGN_FIELD(opts, flr.enable, py_stob)
        ASSIGN_FIELD(opts, flr.line_only, py_stob)
        ASSIGN_FIELD(opts, flr.wireframe, py_stob)
        ASSIGN_FIELD(opts, flr.n_aux, stoi)

        ASSIGN_FIELD(opts, tone.mapper.theme_id, stoi)

        ASSIGN_FIELD(camera, position.x(), stof)
        ASSIGN_FIELD(camera, position.y(), stof)
        ASSIGN_FIELD(camera, position.z(), stof)
        ASSIGN_FIELD(camera, fov_rad, stof)
    }
    cout << "---" << endl;

#undef ASSIGN_FIELD
}

void export_params(
        const rtnpr::Options &opts,
        const rtnpr::Camera &camera,
        nb::dict &dst_dict
) {
    std::string key;

#define ASSIGN_FIELD(trg, field)                     \
key.clear(); key += #trg; key += ":"; key += #field; \
dst_dict[key.c_str()] = trg.field;

    ASSIGN_FIELD(opts, rt.spp_frame)
    ASSIGN_FIELD(opts, rt.spp)
    ASSIGN_FIELD(opts, rt.depth)

    ASSIGN_FIELD(opts, flr.enable)
    ASSIGN_FIELD(opts, flr.line_only)
    ASSIGN_FIELD(opts, flr.normal)
    ASSIGN_FIELD(opts, flr.position)
    ASSIGN_FIELD(opts, flr.wireframe)
    ASSIGN_FIELD(opts, flr.linewidth)
    ASSIGN_FIELD(opts, flr.n_aux)

    ASSIGN_FIELD(opts, tone.map_lines)
    ASSIGN_FIELD(opts, tone.mapper.theme_id)

    ASSIGN_FIELD(camera, position.x())
    ASSIGN_FIELD(camera, position.y())
    ASSIGN_FIELD(camera, position.z())
    ASSIGN_FIELD(camera, fov_rad)

#undef ASSIGN_FIELD
}

template<typename Matrix, typename dtype_>
Matrix to_matrix(nb::tensor<dtype_,nb::shape<nb::any,3>> &nb_tensor)
{
    Matrix m;
    m.resize(nb_tensor.shape(0),3);
    for (int ii = 0; ii < m.rows(); ++ii) {
        m(ii,0) = nb_tensor(ii,0);
        m(ii,1) = nb_tensor(ii,1);
        m(ii,2) = nb_tensor(ii,2);
    }
    return m;
}

} // namespace


NB_MODULE(rtnpr, m) {
    m.def("run_gui", [](
            nb::tensor<float, nb::shape<nb::any, 3>> &V_tensor,
            nb::tensor<int, nb::shape<nb::any, 3>> &F_tensor,
            const nb::dict &params
    ) {
        using namespace std;
        using namespace Eigen;

        auto camera = make_shared<rtnpr::Camera>();
        auto opts = make_shared<rtnpr::Options>();
        import_params(params,*opts,*camera);

        auto V = to_matrix<MatrixXf>(V_tensor);
        auto F = to_matrix<MatrixXi>(F_tensor);

        auto command = run_gui(std::move(V),std::move(F),camera,opts);

        nb::dict out_params;
        export_params(*opts,*camera,out_params);

        out_params["command"] = static_cast<int>(command);
        switch (command) {
            case viewer::Command::None:
            case viewer::Command::RenderImage:
                break;
            case viewer::Command::RenderAnimation:
                out_params["anim:frame_id"] = 0;
                out_params["anim:frames"] = 60;
                break;
        }

        return out_params;
    });


    m.def("run_headless", [](
            nb::tensor<float, nb::shape<nb::any, 3>> &V_tensor,
            nb::tensor<int, nb::shape<nb::any, 3>> &F_tensor,
            const nb::dict &params
    ) {
        using namespace std;
        using namespace Eigen;

        rtnpr::Camera camera;
        rtnpr::Options opts;
        import_params(params,opts,camera);
        camera.look_at(Vector3f::Zero());

        auto V = to_matrix<MatrixXf>(V_tensor);
        auto F = to_matrix<MatrixXi>(F_tensor);

        auto img = run_headless(std::move(V),std::move(F),camera,opts);

        nb::dict out_params;
        export_params(opts,camera,out_params);

        size_t shape[3]{img.width(), img.height(), img.channels()};

        return nb::make_tuple(
                nb::tensor<nb::numpy, float>{img.data(),3, shape},
                out_params
        );
    });
}

#endif // #if defined(RTNPR_NANOBIND)
