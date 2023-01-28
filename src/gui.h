#pragma once

#include "GLFW/glfw3.h"

#include "options.hpp"

namespace rtnpr {

class Gui {
public:
    explicit Gui(GLFWwindow *window) : m_window(window) {}

    void draw();

    [[nodiscard]] const Options &opts() const { return m_opts; }

private:
    GLFWwindow *m_window;

    Options m_opts;

};

} // namespace rtnpr