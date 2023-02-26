#pragma once

#include <memory>

#include "rtnpr/raytracer.h"
#include "rtnpr/camera.hpp"
#include "rtnpr/plane.h"
#include "rtnpr/controls.hpp"

#include "renderer_params.hpp"


namespace viewer {

class Viewer {
public:
    const int width;
    const int height;

    explicit Viewer(int _width = 800, int _height = 800);
    ~Viewer();

    RendererParams open();

    void set_scene(const std::shared_ptr<rtnpr::Scene> &scene);
    void set_opts(const std::shared_ptr<rtnpr::Options> &opts);

private:
    bool m_opened = false;

    class Impl;
    std::unique_ptr<Impl> m_impl;

    rtnpr::RayTracer m_rt;

};

} // namespace viewer