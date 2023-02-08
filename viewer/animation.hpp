#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "rtnpr/camera.hpp"


namespace viewer {

struct KeyFrame {
    rtnpr::Camera camera;
    Eigen::Vector3f light_dir;
};

class Animation {
public:
    std::vector<KeyFrame> keyframes;

    unsigned int temporal_res = 30;
    bool rot_ccw = true;

    void reset() { key_id = 0; frame_id = 0; }

    bool step(rtnpr::Camera &camera, bool logging = false) {
        if (temporal_res == 0) { return false; }
        if (frame_id >= temporal_res) { key_id += 1; }
        if (key_id+1 >= keyframes.size()) { return false; }

        if (logging) {
            using namespace std;
            cout << "Animation: camera pose id = "
                 << key_id*temporal_res+frame_id+1 << " / "
                 << (keyframes.size()-1)*temporal_res << endl;
        }

        const auto &c0 = keyframes[key_id+0].camera;
        const auto &c1 = keyframes[key_id+1].camera;
        const auto res = float(temporal_res);

        float d_radius = (c1.radius-c0.radius)/res;
        float d_phi;
        {
            float c1_phi = c1.phi;
            if (rot_ccw && c1_phi <= c0.phi) { c1_phi += 2.f*float(M_PI); }
            if (!rot_ccw && c1_phi >= c0.phi) { c1_phi -= 2.f*float(M_PI); }
            d_phi = (c1_phi-c0.phi)/res;
        }
        float d_z = (c1.z-c0.z)/res;
        float d_fov = (c1.fov_rad-c0.fov_rad)/res;

        const auto fi = float(frame_id);
        camera.radius = c0.radius + fi * d_radius;
        camera.phi = c0.phi + fi * d_phi;
        camera.z = c0.z + fi * d_z;
        camera.fov_rad = c0.fov_rad + fi * d_fov;

        frame_id += 1;
        return true;
    }


private:
    unsigned int key_id = 0;
    unsigned int frame_id = 0;

};

} // namespace viewer