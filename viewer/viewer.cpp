#include "viewer.h"

#include <iostream>

#if defined(_MSC_VER)
#  include <windows.h>
#endif
//
#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"

#include "imgui.h"

#include "delfem2/opengl/new/drawer_mshtex.h"
#include "delfem2/glfw/viewer3.h"
#include "delfem2/glfw/util.h"

#include "rtnpr/raytracer.h"
#include "gui.h"
#include "tex.hpp"

namespace dfm2 = delfem2;

namespace viewer {

class Viewer::Impl: public dfm2::glfw::CViewer3 {
public:
    std::shared_ptr<rtnpr::Options> opts;
    std::shared_ptr<rtnpr::Camera> camera;
    std::shared_ptr<rtnpr::Scene> scene = std::make_shared<rtnpr::Scene>();
    std::vector<std::shared_ptr<rtnpr::Controls>> controls;


    [[nodiscard]] bool is_ready() const { return opts && camera && scene; }

    Impl() = default;

    ~Impl() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    void InitGL(int _width, int _height, int _tex_width, int _tex_height)
    {
        m_tex.Initialize(_tex_width, _tex_height);
        this->projection = std::make_unique<delfem2::Projection_LookOriginFromZplus>(2, false);
        this->width = _width;
        this->height = _height;

        dfm2::glfw::InitGLNew();
        this->OpenWindow();
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        m_drawer.InitGL();
        m_tex.InitGL();
    }

    void draw(rtnpr::RayTracer &rt, Gui &gui)
    {
        rt.step_gui(m_tex.image, *scene, *camera, *opts);
        m_tex.InitGL();
        //
        ::glfwMakeContextCurrent(this->window);
        ::glClearColor(0.8, 1.0, 1.0, 1.0);
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glEnable(GL_POLYGON_OFFSET_FILL);
        ::glPolygonOffset(1.1f, 4.0f);
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, m_tex.id_tex);
        m_drawer.Draw(dfm2::CMat4f::Identity().data(),
                      dfm2::CMat4f::Identity().data());
        gui.draw();
        this->SwapBuffers();
        glfwPollEvents();
    }

    void CursorPosition(double xpos, double ypos) override
    {
        int width0, height0;
        glfwGetWindowSize(window, &width0, &height0);
        { // update nav
            const double mov_end_x = (2.0 * xpos - width0) / width0;
            const double mov_end_y = (height0 - 2.0 * ypos) / height0;
            nav.dx = mov_end_x - nav.mouse_x;
            nav.dy = mov_end_y - nav.mouse_y;
            nav.mouse_x = mov_end_x;
            nav.mouse_y = mov_end_y;
        }
        if (this->nav.ibutton == GLFW_MOUSE_BUTTON_LEFT) {  // drag for view control
            if (nav.imodifier == GLFW_MOD_SHIFT) {
                for (auto &c: controls) {
                    c->on_horizontal_cursor_move(nav.dx, /*speed=*/.5f);
                    c->on_vertical_cursor_move(nav.dy, /*speed=*/1.f);
                }
                for(const auto& func : this->camerachange_callbacks){ func(); }
                return;
            }
        }
    }

    void mouse_wheel(double yoffset) override {
        for (auto &c: controls) { c->on_mouse_wheel(yoffset,/*speed=*/.5f); }
    }

private:
    dfm2::opengl::Drawer_RectangleTex m_drawer;
    CTexRGB<rtnpr::Image<unsigned char, rtnpr::PixelFormat::RGB>> m_tex;
};

Viewer::Viewer() : m_impl(std::make_unique<Impl>()) {}
Viewer::~Viewer() = default;

bool Viewer::open()
{
    if (m_opened) { return false; }
    if (!m_impl->is_ready()) { return false; }

    auto &opts = *m_impl->opts;
    auto &camera = *m_impl->camera;
    auto &scene = *m_impl->scene;

    using namespace rtnpr;
    using namespace Eigen;

    m_impl->camerachange_callbacks.emplace_back([this]{ this->m_rt.reset(); });
    m_impl->InitGL(width, height, tex_width, tex_height);
    m_opened = true;


    auto camera_controls = std::make_shared<SphereControls<Camera>>();
    camera_controls->set_object(m_impl->camera);
    m_impl->controls.push_back(camera_controls);

    auto light_controls = std::make_shared<UnitDiscControls<Light>>();
    light_controls->set_object(m_impl->scene->light);
    light_controls->enabled = false;
    m_impl->controls.push_back(light_controls);


    auto gui = Gui(m_impl->window);
    bool gui_updated = false;
    auto needs_update = [&gui_updated]() { gui_updated = true; };

    {
        Gui::TreeNode node{"controls"};
        node.add("camera", camera_controls->enabled);
        node.add("light", light_controls->enabled);
        gui.tree_nodes.push_back(std::move(node));
    }

    float back_brightness = 1.f;
    {
        Gui::TreeNode node{"rt"};
        node.add("spp", opts.rt.spp_frame, 1, 64);
        node.add("spp_max", opts.rt.spp, 1, 1024);
        node.add("depth", opts.rt.depth, 1, 8, needs_update);
        node.add("back_brightness", back_brightness, 0.f, 1.f, [&opts, &back_brightness](){
            opts.rt.back_color = Vector3f{1.f,1.f,1.f} * back_brightness;
        });
        gui.tree_nodes.push_back(std::move(node));
    }

    {
        Gui::TreeNode node{"flr"};
        node.open = true;
        node.add("enable", opts.flr.enable, needs_update);
        node.add("line_only", opts.flr.line_only, needs_update);
        node.add("n_aux", opts.flr.n_aux, 4, 16, needs_update);
        node.add("normal", opts.flr.normal, needs_update);
        node.add("position", opts.flr.position, needs_update);
        node.add("wireframe", opts.flr.wireframe, needs_update);
        node.add("width", opts.flr.linewidth, .5f, 5.f, needs_update);
        gui.tree_nodes.push_back(std::move(node));
    }

    {
        Gui::TreeNode node{"plane"};
        node.open = true;
        node.add("visible", scene.plane().visible, needs_update);
        node.add("mat_id", scene.plane().mat_id, 1, 3, needs_update);
        node.add("checkerboard", scene.plane().checkerboard, needs_update);
        node.add("check_res", scene.plane().check_res, 5, 50, needs_update);
        gui.tree_nodes.push_back(std::move(node));
    }

    int map_mode = 1;
    {
        Gui::TreeNode node{"tone"};
        node.add("map_mode", map_mode, 0, 2, [&opts, &map_mode, &gui_updated](){
            opts.tone.map_mode = ToneMapper::MapMode(map_mode);
            gui_updated = true;
        });
        node.add("map_lines", opts.tone.map_lines, needs_update);
        node.add("map_shading", opts.tone.map_shading);
        gui.tree_nodes.push_back(std::move(node));
    }


    glfwSetWindowTitle(m_impl->window, "NPR Viewer");
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(m_impl->window))
    {
        if (gui.capture_and_close) { break; }
        if (gui.anim.add_keyframe) {
            auto l = m_anim.keyframes.empty() ?
                     Eigen::Vector3f(0.f,1.f,1.f) :
                     Eigen::Vector3f(0.f,-1.f,1.f);
            KeyFrame k{*m_impl->camera, l.normalized()};
//            KeyFrame k{*m_impl->camera, m_impl->scene->light->dir()};
            m_anim.keyframes.emplace_back(std::move(k));
        }
        if (gui.anim.clear_keyframe) { m_anim.keyframes.clear(); }
        if (gui.anim.running) {
            m_anim.rot_ccw = gui.anim.rot_ccw;
            gui.anim.running = m_anim.step(*m_impl->camera, *m_impl->scene->light, true);
            gui_updated |= gui.anim.running;
        }
        else {
            m_anim.reset();
        }

        if (gui_updated) { m_rt.reset(); }
        gui_updated = false;
        m_impl->draw(m_rt, gui);

        if (opts.tone.map_shading) {
            opts.tone.mapper.hi_rgb = Vector3f{250.f/255.f,210.f/255.f,219.f/255.f};
            opts.tone.mapper.lo_rgb = Vector3f{165.f/255.f,206.f/255.f,239.f/255.f};
        }
        else {
            opts.tone.mapper.hi_rgb = Vector3f::Ones();
            opts.tone.mapper.lo_rgb = Vector3f::Zero();
        }
    }

    return gui.capture_and_close;
}

void Viewer::set_scene(std::shared_ptr<rtnpr::Scene> &&scene)
{
    m_impl->scene = std::move(scene);
}

void Viewer::set_camera(std::shared_ptr<rtnpr::Camera> &&camera)
{
    m_impl->camera = std::move(camera);
}

void Viewer::set_opts(std::shared_ptr<rtnpr::Options> &&opts)
{
    m_impl->opts = std::move(opts);
}

} // namespace viewer
