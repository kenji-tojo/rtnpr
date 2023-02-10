#include "viewer.h"

#include <iostream>

#if defined(_MSC_VER)
#  include <windows.h>
#endif
//
#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"

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
                for (auto &fn: on_cursor_move_x) { fn(/*movement=*/nav.dx, /*speed=*/.5f); }
                for (auto &fn: on_cursor_move_y) { fn(/*movement=*/nav.dy, /*speed=*/1.f); }
                for(const auto& func : this->camerachange_callbacks){ func(); }
                return;
            }
        }
    }

    void mouse_wheel(double yoffset) override {
        for (auto &fn: on_mouse_wheel) { fn(/*movement=*/yoffset, /*speed=*/.1f); }
    }

    std::vector<std::function<void(float,float)>> on_cursor_move_x;
    std::vector<std::function<void(float,float)>> on_cursor_move_y;
    std::vector<std::function<void(float,float)>> on_mouse_wheel;

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

    using namespace rtnpr;

    m_impl->camerachange_callbacks.emplace_back([this]{ this->m_rt.reset(); });
    m_impl->InitGL(width, height, tex_width, tex_height);
    m_opened = true;


    SphereControls<Camera> camera_controls;
    camera_controls.set_object(m_impl->camera);
    m_impl->on_cursor_move_x.emplace_back([&camera_controls](float movement, float speed){
        camera_controls.on_horizontal_cursor_move(movement, speed);
    });
    m_impl->on_cursor_move_y.emplace_back([&camera_controls](float movement, float speed){
        camera_controls.on_vertical_cursor_move(movement, speed);
    });
    m_impl->on_mouse_wheel.emplace_back([&camera_controls](float movement, float speed){
        camera_controls.on_mouse_wheel(movement, speed);
    });


    UnitDiscControls<Light> light_controls;
    light_controls.enabled = false;
    m_impl->on_cursor_move_x.emplace_back([&light_controls](float movement, float speed){
        light_controls.on_horizontal_cursor_move(movement, speed);
    });
    m_impl->on_cursor_move_y.emplace_back([&light_controls](float movement, float speed){
        light_controls.on_vertical_cursor_move(movement, speed);
    });
    m_impl->on_mouse_wheel.emplace_back([&light_controls](float movement, float speed){
        light_controls.on_mouse_wheel(movement, speed);
    });


    auto gui = Gui(m_impl->window);
    gui.scene = m_impl->scene;
    gui.options = m_impl->opts;


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
            gui.needs_update |= gui.anim.running;
        }
        else {
            m_anim.reset();
        }
        if (gui.needs_update) { m_rt.reset(); }
        m_impl->draw(m_rt, gui);
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
