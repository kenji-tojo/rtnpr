#pragma once

#include "GLFW/glfw3.h"

#include "rtnpr/options.hpp"

namespace viewer {

class Gui {
public:
    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw(rtnpr::Options &opts);

private:

};

} // namespace viewer