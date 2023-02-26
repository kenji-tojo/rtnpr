#pragma once

#include "GLFW/glfw3.h"

#include <vector>
#include <memory>
#include <tuple>
#include <functional>


namespace viewer {

class GuiElement {
public:
    bool sameline = false;
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

        template<bool sameline_ = false>
        void add(const char *label, bool &enabled, std::function<void()> on_update = {});

        template<typename Scalar_, bool sameline_ = false>
        void add(const char *label, Scalar_ &val, Scalar_ min, Scalar_ max, std::function<void()> on_update = {});

        template<bool sameline_ = false>
        void add(const char *label, std::function<void()> on_update);
    };

    std::vector<TreeNode> tree_nodes;
    TreeNode top_level{""};

};


template<bool sameline_>
void Gui::TreeNode::add(const char *label, bool &enabled, std::function<void()> on_update) {
    auto check = std::make_unique<CheckBox>(label, enabled, std::move(on_update));
    if constexpr(sameline_) { check->sameline = true; }
    elements.push_back(std::move(check));
}

template<typename Scalar_, bool sameline_>
void Gui::TreeNode::add(const char *label, Scalar_ &val, Scalar_ min, Scalar_ max, std::function<void()> on_update) {
    constexpr bool is_float = std::is_same_v<Scalar_, float>;
    constexpr bool is_int = std::is_same_v<Scalar_, int>;
    static_assert(is_float || is_int);

    if constexpr(is_float) {
        auto slider = std::make_unique<FloatSlider>(label, val, min, max, std::move(on_update));
        if constexpr(sameline_) { slider->sameline = true; }
        elements.push_back(std::move(slider));
    }
    else if (is_int) {
        auto slider = std::make_unique<IntSlider>(label, val, min, max, std::move(on_update));
        if constexpr(sameline_) { slider->sameline = true; }
        elements.push_back(std::move(slider));
    }
}

template<bool sameline_>
void Gui::TreeNode::add(const char *label, std::function<void()> on_update) {
    auto button = std::make_unique<Button>(label, std::move(on_update));
    if constexpr(sameline_) { button->sameline = true; }
    elements.push_back(std::move(button));
}


} // namespace viewer