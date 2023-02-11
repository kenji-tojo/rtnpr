#pragma once

namespace viewer {

struct RendererParams {
public:
    enum Command: int {
        None = 0,
        RenderImage = 1,
        RenderAnimation = 2,
    };

    int cmd = None;

    struct {
        bool running = false;
        int frame_id = 0;
        int frames = 30;
        struct { bool enabled = true; float step_size = 1.f/30.f; } camera;
        struct { bool enabled = false; float step_size = .5f/30.f; } light;
    } anim;
};

} // namespace viewer