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
        struct { bool enabled = true; float step_size = 1.f; } camera;
        struct { bool enabled = false; float step_size = 1.f; } light;
        int frame_id = 0;
        int frames = 60;
    } anim;
};

} // namespace viewer