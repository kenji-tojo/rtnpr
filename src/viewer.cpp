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
        delfem2::CMat4f mP = this->GetProjectionMatrix();
        delfem2::CMat4f mMV = this->GetModelViewMatrix();
        delfem2::CMat4f mMVP = (mP * mMV).cast<float>();
        rt.step(m_tex.pixel_color, m_tex.width, m_tex.height, mMVP, gui.opts);
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
    }
}

} // namespace rtnpr
