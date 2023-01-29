#pragma once

namespace rtnpr {

struct Options {
public:

    struct {
        int spp = 1;
        int spp_max = 128;
    } rt;

    struct {
        bool normal = false;
        bool position = false;
        bool wireframe = true;
        int n_aux = 8;
    } flr;

};

} // namespace rtnpr