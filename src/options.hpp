#pragma once

#include <memory>
#include <vector>

#include "brdf.hpp"
#include "light.hpp"

namespace rtnpr {

struct Options {
public:
    bool needs_update = false;

    struct {
        int spp_frame = 1;
        int spp = 128;
        int depth = 4;
    } rt;

    struct {
        bool line_only = false;
        bool normal = false;
        bool position = false;
        bool wireframe = false;
        float linewidth = 1.f;
        int n_aux = 4;
        Eigen::Vector3f line_color{50.f/255.f,50.f/255.f,50.f/255.f};
    } flr;

    struct {
        std::vector<std::shared_ptr<BRDF>> brdf = {
                std::make_shared<BRDF>(),
                std::make_shared<SpecularBRDF>()
        };
        std::vector<std::shared_ptr<Light>> light = {
                std::make_shared<Light>(),
                std::make_shared<DirectionalLight>(),
        };
    } scene;

};

} // namespace rtnpr