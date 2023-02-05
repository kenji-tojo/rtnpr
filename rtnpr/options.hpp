#pragma once

#include <memory>
#include <vector>

#include "brdf.hpp"
#include "light.hpp"
#include "plane.h"
#include "tonemapper.hpp"

namespace rtnpr {

struct Options {
public:
    static std::shared_ptr<Options> create() { return std::make_shared<Options>(); }

    bool needs_update = false;
    bool capture_and_close = false;

    struct {
        int spp_frame = 1;
        int spp = 128;
        int depth = 4;
        Eigen::Vector3f back_color{1.f,1.f,1.f};
    } rt;

    struct {
        bool enable = true;
        bool line_only = false;
        bool normal = false;
        bool position = false;
        bool wireframe = true;
        float linewidth = 1.f;
        int n_aux = 4;
        Eigen::Vector3f line_color{93.f/255.f, 63.f/255.f, 221.f/255.f};
    } flr;

    struct {
        std::vector<std::shared_ptr<BRDF>> brdf = {
                std::make_shared<PhongBRDF>(),
                std::make_shared<BRDF>(.2f),
                std::make_shared<GlossyBRDF>(),
                std::make_shared<SpecularBRDF>()
        };

        std::vector<std::shared_ptr<Light>> light = {
                std::make_shared<Light>(),
                std::make_shared<DirectionalLight>(),
        };

        std::shared_ptr<Plane> plane = Plane::create();
    } scene;

    struct {
        ToneMapper mapper;
        ToneMapper::MapMode map_mode = ToneMapper::MapMode::Reinhard;
        bool map_lines = false;
        bool map_shading = true;
    } tone;

};

} // namespace rtnpr