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

    void add(bool *enabled, const char *title) {
        CheckBox e{enabled, title};
        m_checks.emplace_back(e);
    }

    template<typename Scalar_>
    void add(Scalar_ *val, Scalar_ min, Scalar_ max, const char *title) {
        constexpr bool is_float_val = std::is_same_v<Scalar_, float>;
        constexpr bool is_int_val = std::is_same_v<Scalar_, int>;
        static_assert(is_float_val || is_int_val);
        Slider<Scalar_> e{val, min, max, title};
        if constexpr(is_float_val) { m_float_sliders.emplace_back(e); }
        else if (is_int_val) { m_int_sliders.emplace_back(e); }
    }

    struct CheckBox {
        bool *enabled;
        const char *title;
    };

    template<typename Scalar_>
    struct Slider {
        Scalar_ *val;
        Scalar_ min;
        Scalar_ max;
        const char *title;
    };

private:
    std::vector<CheckBox> m_checks;
    std::vector<Slider<float>> m_float_sliders;
    std::vector<Slider<int>> m_int_sliders;

};

} // namespace viewer