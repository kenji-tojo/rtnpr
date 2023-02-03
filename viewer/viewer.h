#pragma once

#include <memory>

#include "rtnpr/raytracer.h"
#include "rtnpr/camera.hpp"
#include "rtnpr/plane.h"

namespace viewer {

class Viewer {
public:
    int tex_width = 128;
    int tex_height = 128;

    int width = 800;
    int height = 800;

    Viewer();
    ~Viewer();

    void open();

    void set_scene(rtnpr::Scene scene);

private:
    bool m_opened = false;

    class Impl;
    std::unique_ptr<Impl> m_impl;

    rtnpr::RayTracer m_rt;
    std::shared_ptr<rtnpr::Plane> m_plane = rtnpr::Plane::create();

};

} // namespace viewer