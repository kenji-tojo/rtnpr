#pragma once

#include "GLFW/glfw3.h"

#include "rtnpr/options.hpp"


namespace viewer {

class Gui {
public:
    bool needs_update = false;
    bool capture_and_close = false;

    struct {
        int n_kf = 0;
        bool add_keyframe = false;
        bool clear_keyframe = false;
        bool running = false;
        void reset() {
            add_keyframe = false;
            clear_keyframe = false;
        }
    } anim;

    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw(rtnpr::Options &opts);

private:

};

} // namespace viewer