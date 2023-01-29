#pragma once

namespace rtnpr {

struct Options {
public:

    struct {
        int spp = 1;
        int spp_max = 1024;
        int n_aux = 8;
    } rt;

};

} // namespace rtnpr