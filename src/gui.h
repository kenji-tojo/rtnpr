#pragma once

#include "GLFW/glfw3.h"

#include "options.hpp"

namespace rtnpr {

class Gui {
public:
    Options opts;

    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw();

private:

};

} // namespace rtnpr