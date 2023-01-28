#pragma once

#include <memory>

#include "raytracer.h"

namespace rtnpr {

class Viewer {
public:
    int tex_width = 256;
    int tex_height = 256;

    int width = 800;
    int height = 800;

    Viewer();
    ~Viewer();

    void open();

private:
    bool m_opened = false;

    class Impl;
    std::unique_ptr<Impl> m_impl;

    RayTracer m_rt;

};

} // namespace rtnpr