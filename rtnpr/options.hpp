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

#if defined(NDEBUG)
    static constexpr int default_width = 800;
    static constexpr int default_height = 800;
#else
    static constexpr int default_width = 128;
    static constexpr int default_height = 128;
#endif

    static std::shared_ptr<Options> create() { return std::make_shared<Options>(); }

    struct {
        int width = default_width;
        int height = default_height;
    } img;

    struct {
        int spp_frame = 1;
        int spp = 128;
        int depth = 4;
        Eigen::Vector3f back_color{1.f,1.f,1.f};
        bool surface_normal = false;
    } rt;

    struct {
        bool enable = true;
        bool line_only = false;
        bool normal = false;
        bool position = false;
        bool wireframe = false;
        float width = 1.f;
        int n_aux = 4;
        Eigen::Vector3f line_color{93.f/255.f, 63.f/255.f, 221.f/255.f};
    } flr;

    struct {
        ToneMapper mapper;
        int theme_id = 0;
    } tone;

};

} // namespace rtnpr