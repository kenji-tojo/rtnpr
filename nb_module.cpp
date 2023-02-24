#include <iostream>
#include <memory>
#include <string>

#include "viewer/viewer.h"
#include "rtnpr/trimesh.h"

#if defined (RTNPR_TEST)
#include <igl/readOBJ.h>
#endif

#if defined(RTNPR_NANOBIND)
#include <nanobind/nanobind.h>
#include <nanobind/tensor.h>
#endif


using namespace rtnpr;

namespace {

viewer::RendererParams run_gui(
        std::shared_ptr<Scene> scene,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Options> opts
) {
    viewer::Viewer viewer;
    viewer.set_scene(std::move(scene));
    viewer.set_camera(std::move(camera));
    viewer.set_opts(std::move(opts));
    return viewer.open();
}


Image<float, PixelFormat::RGBA> run_headless(
        const Scene &scene,
        const Camera &camera,
        const Options &opts
) {
    using namespace std;

    const int spp = opts.rt.spp;
    const int spp_frame = opts.rt.spp_frame;

    const int width = opts.img.width;
    const int height = opts.img.height;

    RayTracer rt;
    for (int ii = 0; ii < spp/spp_frame; ++ii) {
        rt.step_headless(width, height, scene, camera, opts);
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

    auto scene = make_shared<Scene>();
    {
        MatrixXf V;
        MatrixXi F;
        igl::readOBJ("assets/bunny.obj",V,F);
        scene->add(TriMesh::create(V,F));
    }

    auto camera = make_shared<rtnpr::Camera>();
    camera->position = Vector3f(0.f,-135.f,80.f);

    run_gui(scene,camera, make_shared<Options>());
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
        const nb::dict &params_dict,
        Scene &scene,
        Camera &camera,
        Options &opts,
        viewer::RendererParams &renderer_params
) {
    using namespace std;

#define ASSIGN_FIELD(trg, field, cast_fn)                 \
if (key == #field) {                                      \
    trg.field = cast_fn(value);                           \
    cout << #field << " = " << ::fmt_(trg.field) << endl; \
}

    cout << "--- importing options ---" << endl;
    for (const auto &item: params_dict)
    {
        auto key = string(nb::str(item.first).c_str());
        auto value = string(nb::str(item.second).c_str());

        auto pos = key.find(':');
        if (pos != string::npos) {
            cout << key.substr(0,pos) << ".";
            key = key.substr(pos+1,key.length());
        }

        ASSIGN_FIELD(opts, img.width, stoi)
        ASSIGN_FIELD(opts, img.height, stoi)

        ASSIGN_FIELD(opts, rt.spp_frame, stoi)
        ASSIGN_FIELD(opts, rt.spp, stoi)
        ASSIGN_FIELD(opts, rt.depth, stoi)

        ASSIGN_FIELD(opts, flr.intensity, stof)
        ASSIGN_FIELD(opts, flr.width, stof)
        ASSIGN_FIELD(opts, flr.enable, py_stob)
        ASSIGN_FIELD(opts, flr.line_only, py_stob)
        ASSIGN_FIELD(opts, flr.wireframe, py_stob)
        ASSIGN_FIELD(opts, flr.n_aux, stoi)

        ASSIGN_FIELD(opts, tone.mapper.theme_id, stoi)
        ASSIGN_FIELD(opts, tone.map_lines, py_stob)


        ASSIGN_FIELD(camera, position.x(), stof)
        ASSIGN_FIELD(camera, position.y(), stof)
        ASSIGN_FIELD(camera, position.z(), stof)
        ASSIGN_FIELD(camera, fov_rad, stof)


        ASSIGN_FIELD(scene, light->position.x(), stof)
        ASSIGN_FIELD(scene, light->position.y(), stof)
        ASSIGN_FIELD(scene, light->position.z(), stof)

        ASSIGN_FIELD(scene, plane().mat_id, stoi)
        ASSIGN_FIELD(scene, plane().check_res, stoi)
        ASSIGN_FIELD(scene, plane().checkerboard, py_stob)


        ASSIGN_FIELD(renderer_params, cmd, stoi)

        ASSIGN_FIELD(renderer_params, anim.running, py_stob)
        ASSIGN_FIELD(renderer_params, anim.frames, stoi)
        ASSIGN_FIELD(renderer_params, anim.frame_id, stoi)

        ASSIGN_FIELD(renderer_params, anim.camera.enabled, py_stob)
        ASSIGN_FIELD(renderer_params, anim.camera.step_size, stof)

        ASSIGN_FIELD(renderer_params, anim.light.enabled, py_stob)
        ASSIGN_FIELD(renderer_params, anim.light.step_size, stof)
    }
    cout << "---" << endl;

#undef ASSIGN_FIELD
}

void export_params(
        const Scene &scene,
        const Camera &camera,
        const Options &opts,
        const viewer::RendererParams &renderer_params,
        nb::dict &params_dict
) {
    std::string key;

#define ASSIGN_FIELD(trg, field)                     \
key.clear(); key += #trg; key += ":"; key += #field; \
params_dict[key.c_str()] = trg.field;

    ASSIGN_FIELD(opts, img.width)
    ASSIGN_FIELD(opts, img.height)

    ASSIGN_FIELD(opts, rt.spp_frame)
    ASSIGN_FIELD(opts, rt.spp)
    ASSIGN_FIELD(opts, rt.depth)

    ASSIGN_FIELD(opts, flr.enable)
    ASSIGN_FIELD(opts, flr.line_only)
    ASSIGN_FIELD(opts, flr.normal)
    ASSIGN_FIELD(opts, flr.position)
    ASSIGN_FIELD(opts, flr.wireframe)
    ASSIGN_FIELD(opts, flr.intensity)
    ASSIGN_FIELD(opts, flr.width)
    ASSIGN_FIELD(opts, flr.n_aux)

    ASSIGN_FIELD(opts, tone.mapper.theme_id)
    ASSIGN_FIELD(opts, tone.map_lines)


    ASSIGN_FIELD(camera, position.x())
    ASSIGN_FIELD(camera, position.y())
    ASSIGN_FIELD(camera, position.z())
    ASSIGN_FIELD(camera, fov_rad)


    ASSIGN_FIELD(scene, light->position.x())
    ASSIGN_FIELD(scene, light->position.y())
    ASSIGN_FIELD(scene, light->position.z())

    ASSIGN_FIELD(scene, plane().mat_id)
    ASSIGN_FIELD(scene, plane().check_res)
    ASSIGN_FIELD(scene, plane().checkerboard)


    ASSIGN_FIELD(renderer_params, cmd)

    ASSIGN_FIELD(renderer_params, anim.running)
    ASSIGN_FIELD(renderer_params, anim.frames)
    ASSIGN_FIELD(renderer_params, anim.frame_id)

    ASSIGN_FIELD(renderer_params, anim.camera.enabled)
    ASSIGN_FIELD(renderer_params, anim.camera.step_size)

    ASSIGN_FIELD(renderer_params, anim.light.enabled)
    ASSIGN_FIELD(renderer_params, anim.light.step_size)

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
            nb::tensor<float, nb::shape<nb::any, 3>> &V,
            nb::tensor<int, nb::shape<nb::any, 3>> &F,
            const nb::dict &params
    ) {
        using namespace std;
        using namespace Eigen;

        viewer::RendererParams renderer_params;
        auto scene = make_shared<Scene>();
        auto camera = make_shared<Camera>();
        auto opts = make_shared<Options>();

        import_params(params,*scene,*camera,*opts,renderer_params);
        scene->add(TriMesh::create(to_matrix<MatrixXf>(V), to_matrix<MatrixXi>(F)));

        renderer_params = run_gui(scene,camera,opts);

        nb::dict out_params;
        export_params(*scene,*camera,*opts,renderer_params,out_params);

        return out_params;
    });


    m.def("run_headless", [](
            nb::tensor<float, nb::shape<nb::any, 3>> &V,
            nb::tensor<int, nb::shape<nb::any, 3>> &F,
            const nb::dict &params
    ) {
        using namespace std;
        using namespace Eigen;

        viewer::RendererParams renderer_params;
        auto camera = std::make_shared<Camera>();
        Options opts;
        Scene scene;

        import_params(params,scene,*camera,opts,renderer_params);
        scene.add(TriMesh::create(to_matrix<MatrixXf>(V), to_matrix<MatrixXi>(F)));


        switch (renderer_params.cmd) {
            using Command = viewer::RendererParams::Command;
            case Command::RenderImage: {
                camera->look_at(Vector3f::Zero());
                renderer_params.cmd = Command::None;
                break;
            }
            case Command::RenderAnimation: {
                auto &anim = renderer_params.anim;

                SphereControls<Camera> cc;
                cc.set_object(camera);
                cc.enabled = anim.camera.enabled;

                UnitDiscControls<Light> lc;
                lc.set_object(scene.light);
                lc.enabled = anim.light.enabled;

                anim.frame_id += 1;
                if (anim.frame_id > 1) {
                    cc.on_horizontal_cursor_move(anim.camera.step_size,-1.f);
                    lc.on_horizontal_cursor_move(anim.light.step_size,-1.f);
                }
                if (anim.frame_id >= anim.frames) {
                    renderer_params.cmd = Command::None;
                    anim.frame_id = 0;
                    anim.running = false;
                }
                break;
            }
            default: assert(false);
        }


        auto img = run_headless(scene,*camera,opts);

        nb::dict out_params;
        export_params(scene,*camera,opts,renderer_params,out_params);

        size_t shape[3]{img.width(), img.height(), img.channels()};

        return nb::make_tuple(
                nb::tensor<nb::numpy, float>{img.data(),3, shape},
                out_params
        );
    });
}

#endif // #if defined(RTNPR_NANOBIND)
