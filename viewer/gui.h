#pragma once

#include "GLFW/glfw3.h"

#include "rtnpr/options.hpp"

namespace viewer {

class Gui {
public:
    rtnpr::Options opts;

    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw();

private:

};

} // namespace viewer