#pragma once

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
        float linewidth = .7f;
        int n_aux = 12;
    } flr;

};

} // namespace rtnpr