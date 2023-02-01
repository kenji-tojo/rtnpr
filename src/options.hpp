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
        int spp_frame = 2;
        int spp = 128;
        int depth = 4;
    } rt;

    struct {
        bool normal = false;
        bool position = false;
        bool wireframe = false;
        float linewidth = 1.f;
        int n_aux = 4;
    } flr;

    struct {
        std::vector<std::shared_ptr<BRDF>> brdf = {std::make_shared<BRDF>()};
        std::vector<std::shared_ptr<Light>> light = {
                std::make_shared<Light>(),
                std::make_shared<DirectionalLight>(),
        };
    } scene;

};

} // namespace rtnpr