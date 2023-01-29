#pragma once

namespace rtnpr {

struct Options {
public:
    bool needs_update = false;

    struct {
        int spp = 1;
        int spp_max = 128;
    } rt;

    struct {
        bool normal = false;
        bool position = false;
        bool wireframe = true;
        float linewidth = 2.f;
        int n_aux = 8;
    } flr;

};

} // namespace rtnpr