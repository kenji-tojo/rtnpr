#include <iostream>
#include <memory>

#include "viewer/viewer.h"
#include "rtnpr/trimesh.h"
#include "rtnpr/thread.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/tensor.h>


namespace nb = nanobind;
using namespace nb::literals;


namespace {

template<typename Matrix, typename dtype_>
Matrix to_matrix(nb::tensor<dtype_,nb::shape<nb::any,3>> &nb_tensor) {
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


#define DEFINE_GETTER_AND_SETTER(scope, name, prefix, type) \
void set_##scope##name(type name) { (prefix).name = name; } \
type get_##scope##name() const { return (prefix).name; }

namespace rtnpr::binding {

class NbCamera {
public:
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();

    void set_position(float x, float y, float z) const {
        assert(camera);
        camera->position = Eigen::Vector3f{x,y,z};
    }

    [[nodiscard]] nb::tuple get_position() const {
        assert(camera);
        const auto &p = camera->position;
        return nb::make_tuple(p[0], p[1], p[2]);
    };

    DEFINE_GETTER_AND_SETTER(, fov, *camera, float)

    void look_at(float x, float y, float z) const {
        assert(camera);
        Eigen::Vector3f target{x,y,z};
        camera->look_at(target);
    }
};


class NbLight {
public:
    enum Type {
        SoftDirectional = 0,
        Directional     = 1
    };

    std::shared_ptr<Light> light;

    NbLight() {
        light = std::make_shared<Light>();
    }

    explicit NbLight(Type type) {
        switch (type) {
            case Directional:
                light = std::make_shared<DirectionalLight>();
                break;
            case SoftDirectional:
            default:
                light = std::make_shared<Light>();
                break;
        }
    }

    void set_position(float x, float y, float z) const {
        assert(light);
        light->position = Eigen::Vector3f{x,y,z};
    }

    [[nodiscard]] nb::tuple get_position() const {
        assert(light);
        const auto &p = light->position;
        return nb::make_tuple(p[0], p[1], p[2]);
    };

    void look_at(float x, float y, float z) const {
        assert(light);
        Eigen::Vector3f target{x,y,z};
        light->look_at(target);
    }

    void set_dir(float x, float y, float z) const {
        assert(light);
        Eigen::Vector3f dir{x,y,z};
        light->set_dir(dir);
    }

    [[nodiscard]] nb::tuple get_dir() const {
        assert(light);
        const auto &d = light->dir();
        return nb::make_tuple(d[0], d[1], d[2]);
    };
};


class NbMesh {
public:
    std::shared_ptr<TriMesh> mesh;

    NbMesh(nb::tensor<float, nb::shape<nb::any, 3>> &V, nb::tensor<int, nb::shape<nb::any, 3>> &F) {
        using namespace Eigen;
        mesh = std::make_shared<TriMesh>(to_matrix<MatrixXf>(V), to_matrix<MatrixXi>(F));
    }

    DEFINE_GETTER_AND_SETTER(, visible, *mesh, bool)

    void set_shift_z(float shift_z) const { assert(mesh); mesh->transform->shift.z() = shift_z; mesh->apply_transform(); }
    [[nodiscard]] float get_shift_z() const { return mesh->transform->shift.z(); }

    void set_scale(float scale) const { assert(mesh); mesh->transform->scale = scale; mesh->apply_transform(); }
    [[nodiscard]] float get_scale() const { return mesh->transform->scale; }
};



class NbScene {
public:
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    viewer::RendererParams params;

    void set_camera(const NbCamera &camera) const {
        assert(scene);
        scene->camera = camera.camera;
    }

    void set_light(const NbLight &light) const {
        assert(scene);
        scene->light = light.light;
    }

    void add_mesh(const NbMesh &mesh) const {
        assert(scene);
        scene->add(mesh.mesh);
    }

    DEFINE_GETTER_AND_SETTER(plane_, mat_id, scene->plane(), int)
    DEFINE_GETTER_AND_SETTER(plane_, checkerboard, scene->plane(), bool)
    DEFINE_GETTER_AND_SETTER(plane_, check_res, scene->plane(), int)
    DEFINE_GETTER_AND_SETTER(plane_, albedo, *scene->brdf[scene->plane().mat_id], float)
    DEFINE_GETTER_AND_SETTER(plane_, visible, scene->plane(), bool)
    DEFINE_GETTER_AND_SETTER(plane_, transparent, scene->plane(), bool)

    DEFINE_GETTER_AND_SETTER(phong_, kd, scene->phong(), float)
    DEFINE_GETTER_AND_SETTER(phong_, power, scene->phong().glossy, int)
    DEFINE_GETTER_AND_SETTER(phong_, albedo, scene->phong(), float)

    [[nodiscard]] int get_command() const { return int(params.cmd); }

    [[nodiscard]] int get_frame_id() const { return params.anim.frame_id; }
    [[nodiscard]] int get_frames() const { return params.anim.frames; }
    [[nodiscard]] bool is_light_animated() const { return params.anim.light.enabled; }
    [[nodiscard]] bool is_camera_animated() const { return params.anim.camera.enabled; }
};


class NbOptions {
public:
    std::shared_ptr<Options> options = std::make_shared<Options>();

    DEFINE_GETTER_AND_SETTER(img_, width, options->img, int)
    DEFINE_GETTER_AND_SETTER(img_, height, options->img, int)

    DEFINE_GETTER_AND_SETTER(rt_, spp_frame, options->rt, int)
    DEFINE_GETTER_AND_SETTER(rt_, spp, options->rt, int)
    DEFINE_GETTER_AND_SETTER(rt_, depth, options->rt, int)
    DEFINE_GETTER_AND_SETTER(rt_, surface_normal, options->rt, bool)

    DEFINE_GETTER_AND_SETTER(flr_, width, options->flr, float)
    DEFINE_GETTER_AND_SETTER(flr_, enable, options->flr, bool)
    DEFINE_GETTER_AND_SETTER(flr_, line_only, options->flr, bool)
    DEFINE_GETTER_AND_SETTER(flr_, wireframe, options->flr, bool)
    DEFINE_GETTER_AND_SETTER(flr_, n_aux, options->flr, int)

    DEFINE_GETTER_AND_SETTER(tone_, mode, options->tone.mapper, int)
    DEFINE_GETTER_AND_SETTER(tone_, theme_id, options->tone, int)
};


} // namespace rtnpr::binding


using namespace rtnpr;


namespace {

void render(
        const Scene &scene,
        const Options &opts,
        nb::tensor<float, nb::shape<nb::any, nb::any, 4>> &img_dest
) {
    using namespace std;

    const int spp = opts.rt.spp;
    const int spp_frame = opts.rt.spp_frame;

    RayTracer rt;
    for (int ii = 0; ii < spp/spp_frame; ++ii) {
        rt.step</*write_image=*/false>(img_dest, scene, opts);
        cout << "spp: " << rt.spp() << endl;
    }

    rt.write</*flip_axis=*/true>(img_dest, opts);
}

} // namespace


using namespace rtnpr::binding;

#define DEFINE_PROPERTY(prefix, name) .def_property(#name, &prefix::get_##name, &prefix::set_##name)

NB_MODULE(rtnpr, m) {

    nb::class_<NbCamera>(m, "Camera")
            .def(nb::init<>())
            .def("set_position", &NbCamera::set_position)
            .def_property_readonly("position", &NbCamera::get_position)
            DEFINE_PROPERTY(NbCamera, fov)
            .def("look_at", &NbCamera::look_at);

    nb::class_<NbLight> light(m, "Light");

    light.def(nb::init<NbLight::Type>())
            .def("set_position", &NbLight::set_position)
            .def_property_readonly("position", &NbLight::get_position)
            .def("set_dir", &NbLight::set_dir)
            .def_property_readonly("dir", &NbLight::get_dir)
            .def("look_at", &NbLight::look_at);

    nb::enum_<NbLight::Type>(light, "Type")
            .value("SoftDirectional", NbLight::SoftDirectional)
            .value("Directional", NbLight::Directional)
            .export_values();

    nb::class_<NbMesh>(m, "TriMesh")
            .def(nb::init<nb::tensor<float, nb::shape<nb::any, 3>> &, nb::tensor<int, nb::shape<nb::any, 3>> &>())
            DEFINE_PROPERTY(NbMesh, visible)
            DEFINE_PROPERTY(NbMesh, shift_z)
            DEFINE_PROPERTY(NbMesh, scale);

    nb::class_<NbScene>(m, "Scene")
            .def(nb::init<>())
            .def("set_camera", &NbScene::set_camera)
            .def("set_light", &NbScene::set_light)
            .def("add_mesh", &NbScene::add_mesh)
            .def("get_command", &NbScene::get_command)
            .def("get_frame_id", &NbScene::get_frame_id)
            .def("get_frames", &NbScene::get_frames)
            .def("is_light_animated", &NbScene::is_light_animated)
            .def("is_camera_animated", &NbScene::is_camera_animated)
            DEFINE_PROPERTY(NbScene, phong_kd)
            DEFINE_PROPERTY(NbScene, phong_power)
            DEFINE_PROPERTY(NbScene, phong_albedo)
            DEFINE_PROPERTY(NbScene, plane_mat_id)
            DEFINE_PROPERTY(NbScene, plane_checkerboard)
            DEFINE_PROPERTY(NbScene, plane_check_res)
            DEFINE_PROPERTY(NbScene, plane_albedo)
            DEFINE_PROPERTY(NbScene, plane_visible)
            DEFINE_PROPERTY(NbScene, plane_transparent);

    nb::class_<NbOptions>(m, "Options")
            .def(nb::init<>())
            DEFINE_PROPERTY(NbOptions, img_width)
            DEFINE_PROPERTY(NbOptions, img_height)
            DEFINE_PROPERTY(NbOptions, rt_spp_frame)
            DEFINE_PROPERTY(NbOptions, rt_spp)
            DEFINE_PROPERTY(NbOptions, rt_surface_normal)
            DEFINE_PROPERTY(NbOptions, rt_depth)
            DEFINE_PROPERTY(NbOptions, flr_width)
            DEFINE_PROPERTY(NbOptions, flr_enable)
            DEFINE_PROPERTY(NbOptions, flr_line_only)
            DEFINE_PROPERTY(NbOptions, flr_wireframe)
            DEFINE_PROPERTY(NbOptions, flr_n_aux)
            DEFINE_PROPERTY(NbOptions, tone_mode)
            DEFINE_PROPERTY(NbOptions, tone_theme_id);

    m.def("show", [] (NbScene &scn, NbOptions &opts) {
        auto scene = scn.scene;
        auto options = opts.options;

        viewer::Viewer viewer;
        viewer.set_scene(scn.scene);
        viewer.set_opts(opts.options);
        scn.params = viewer.open();
    });

    m.def("create_camera_positions", [] (
            NbScene &scn, nb::tensor<float, nb::shape<nb::any, 3>> &positions
    ) {
        const auto anim = scn.params.anim;

        if (!anim.camera.enabled || positions.shape(0) != anim.frames) {
            std::cerr << "error: return without setting camera positions" << std::endl;
            return;
        }

        auto &camera = scn.scene->camera;

        SphereControls<Camera> cc;
        cc.set_object(camera);

        for (int frame_id = 0; frame_id < anim.frames; ++frame_id) {
            if (frame_id > 0)
                cc.on_horizontal_cursor_move(anim.camera.step_size, -1.f);
            positions(frame_id, 0) = camera->position.x();
            positions(frame_id, 1) = camera->position.y();
            positions(frame_id, 2) = camera->position.z();
        }
    });

    m.def("render", [] (
            NbScene &scn,
            NbOptions &opts,
            nb::tensor<float, nb::shape<nb::any, nb::any, 4>> &img_dest
    ) {
        using namespace Eigen;

        if (!scn.scene) {
            std::cerr << "error: scene is empty" << std::endl;
            return;
        }

        if (!opts.options) {
            std::cerr << "options are empty. using defalt values" << std::endl;
            opts.options = std::make_shared<Options>();
        }

        if (!scn.scene->camera) {
            std::cerr << "error: camera is not set" << std::endl;
            return;
        }

        std::cout << "rendering image with size "
                  << img_dest.shape(0) << "x"
                  << img_dest.shape(1) << "x"
                  << img_dest.shape(2) << std::endl;

        render(*scn.scene, *opts.options, img_dest);
    });

    m.def("tone_map", [] (
            nb::tensor<float, nb::shape<nb::any, nb::any, 4>> &img,
            int map_mode,
            int theme_id
    ) {
        const unsigned int width  = img.shape(0);
        const unsigned int height = img.shape(1);

        const unsigned int n_threads = std::thread::hardware_concurrency();

        using rtnpr::ToneMapper;
        std::vector<ToneMapper> mapper;
        mapper.resize(n_threads);
        for (auto &m: mapper)
            m.mode = map_mode;

        rtnpr::parallel_for(width, height, [&] (int iw, int ih, int tid) {
            Eigen::Vector3f rgb;
            rgb[0] = img(iw, ih, 0);
            rgb[1] = img(iw, ih, 1);
            rgb[2] = img(iw, ih, 2);
            rgb = mapper[tid].map3(rgb, theme_id);
            img(iw, ih, 0) = rgb[0];
            img(iw, ih, 1) = rgb[1];
            img(iw, ih, 2) = rgb[2];
        }, n_threads);
    });

}
