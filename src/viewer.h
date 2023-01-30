#pragma once

#include <memory>

#include "raytracer.h"
#include "camera.hpp"

namespace rtnpr {

class Viewer {
public:
    int tex_width = 128;
    int tex_height = 128;

    int width = 800;
    int height = 800;

    Viewer();
    ~Viewer();

    void open();

    void set_scene(Scene scene)
    {
        m_rt.scene = std::move(scene);
    }

private:
    bool m_opened = false;

    class Impl;
    std::unique_ptr<Impl> m_impl;

    RayTracer m_rt;

};

} // namespace rtnpr