#pragma once

#include "GLFW/glfw3.h"

#include "rtnpr/options.hpp"
#include "rtnpr/scene.hpp"


namespace viewer {

class Gui {
public:
    std::shared_ptr<rtnpr::Options> options;
    std::shared_ptr<rtnpr::Scene> scene;

    bool needs_update = false;
    bool capture_and_close = false;

    struct {
        int n_kf = 0;
        bool add_keyframe = false;
        bool clear_keyframe = false;
        bool running = false;
        bool rot_ccw = true;
        void reset() {
            add_keyframe = false;
            clear_keyframe = false;
        }
    } anim;

    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw();

private:

};

} // namespace viewer