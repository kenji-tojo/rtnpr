#pragma once

#include "GLFW/glfw3.h"

#include <vector>
#include <memory>
#include <tuple>
#include <functional>


namespace viewer {

class GuiElement {
public:
    virtual void draw() {}
};

class CheckBox: public GuiElement {
public:
    CheckBox(const char *_label, bool &_enabled, std::function<void()> &&_on_update)
            : label(_label), enabled(_enabled), on_update(std::move(_on_update)) {}
    void draw() override;

private:
    bool &enabled;
    const char *label;
    std::function<void()> on_update;
};

class IntSlider: public GuiElement {
public:
    IntSlider(const char *_label, int &_val, int _min, int _max, std::function<void()> &&_on_update)
            : label(_label), val(_val), min(_min), max(_max), on_update(std::move(_on_update)) {}
    void draw() override;

private:
    int &val;
    int min, max;
    const char *label;
    std::function<void()> on_update;
};

class FloatSlider: public GuiElement {
public:
    FloatSlider(const char *_label, float &_val, float _min, float _max, std::function<void()> &&_on_update)
            : label(_label), val(_val), min(_min), max(_max), on_update(std::move(_on_update)) {}
    void draw() override;

private:
    float &val;
    float min, max;
    const char *label;
    std::function<void()> on_update;
};

class Button: public GuiElement {
public:
    Button(const char *_label, std::function<void()> &&_on_update)
            : label(_label), on_update(_on_update) {}
    void draw() override;

private:
    const char *label;
    std::function<void()> on_update;
};


class Gui {
public:
    explicit Gui(GLFWwindow *window);
    ~Gui();

    void draw();


    struct TreeNode {
        explicit TreeNode(const char *_label): label(_label) {}

        bool open = false;
        const char *label;
        std::vector<std::unique_ptr<GuiElement>> elements;

        void add(const char *label, bool &enabled, std::function<void()> on_update = {})
        {
            auto check = std::make_unique<CheckBox>(label, enabled, std::move(on_update));
            elements.push_back(std::move(check));
        }

        template<typename Scalar_>
        void add(const char *label, Scalar_ &val, Scalar_ min, Scalar_ max, std::function<void()> on_update = {})
        {
            constexpr bool is_float_val = std::is_same_v<Scalar_, float>;
            constexpr bool is_int_val = std::is_same_v<Scalar_, int>;
            static_assert(is_float_val || is_int_val);

            if constexpr(is_float_val) {
                auto slider = std::make_unique<FloatSlider>(label, val, min, max, std::move(on_update));
                elements.push_back(std::move(slider));
            }
            else if (is_int_val) {
                auto slider = std::make_unique<IntSlider>(label, val, min, max, std::move(on_update));
                elements.push_back(std::move(slider));
            }
        }

        void add(const char *label, std::function<void()> on_update)
        {
            auto button = std::make_unique<Button>(label, std::move(on_update));
            elements.push_back(std::move(button));
        }
    };

    std::vector<TreeNode> tree_nodes;
    TreeNode top_level{""};

};

} // namespace viewer