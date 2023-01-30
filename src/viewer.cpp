#include "viewer.h"

#include <cstdio>
#include <iostream>

#if defined(_MSC_VER)
#  include <windows.h>
#endif
//
#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"

#include "delfem2/opengl/new/drawer_mshtex.h"
#include "delfem2/opengl/tex.h"
#include "delfem2/msh_io_ply.h"
#include "delfem2/glfw/viewer3.h"
#include "delfem2/glfw/util.h"

#include "raytracer.h"
#include "gui.h"

namespace dfm2 = delfem2;

namespace rtnpr {

class Viewer::Impl: public dfm2::glfw::CViewer3 {
public:
    Camera camera;

    Impl() = default;

    ~Impl() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    void InitGL(int _width, int _height, int _tex_width, int _tex_height)
    {
        m_tex.width = _tex_width;
        m_tex.height = _tex_height;
        m_tex.channels = 3;
        m_tex.pixel_color.resize(m_tex.width * m_tex.height * m_tex.channels);
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

    void draw(RayTracer &rt, Gui &gui)
    {
        rt.step(m_tex.pixel_color, m_tex.width, m_tex.height, camera, gui.opts);
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
            if (nav.imodifier == GLFW_MOD_ALT) {
                return;
            } else if (nav.imodifier == GLFW_MOD_SHIFT) {
                camera.shift_z(nav.dy);
                camera.shift_phi(.5f*nav.dx);
                for(const auto& func : this->camerachange_callbacks){ func(); }
                return;
            }
        }
    }

    void mouse_wheel(double yoffset) override
    {
        camera.shift_radius(.1f*yoffset);
    }

private:
    dfm2::opengl::Drawer_RectangleTex m_drawer;
    dfm2::opengl::CTexRGB_Rect2D m_tex;

};

Viewer::Viewer() : m_impl(std::make_unique<Impl>()) {}
Viewer::~Viewer() = default;

void Viewer::open()
{
    if (m_opened) { return; }

    m_impl->camerachange_callbacks.emplace_back([this]{ this->m_rt.reset(); });
    m_impl->InitGL(width, height, tex_width, tex_height);
    m_opened = true;

    auto gui = Gui(m_impl->window);

    glfwSetWindowTitle(m_impl->window, "NPR Viewer");
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(m_impl->window))
    {
        m_impl->draw(m_rt, gui);
        if (gui.opts.needs_update)
        {
            m_rt.reset();
        }
    }
}

} // namespace rtnpr
