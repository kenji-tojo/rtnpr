#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "rtnpr/camera.hpp"


namespace viewer {

class Animation {
public:
    std::vector<rtnpr::Camera> keyframes;

    unsigned int temporal_res = 30;
    float rot_dir = 1.f;

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

        const auto &k0 = keyframes[key_id+0];
        const auto &k1 = keyframes[key_id+1];
        const auto res = float(temporal_res);

        float d_radius = (k1.radius-k0.radius)/res;
        float d_phi = (k1.phi-k0.phi)/res;
        float d_z = (k1.z-k0.z)/res;
        float d_fov = (k1.fov_rad-k0.fov_rad)/res;

        const auto fi = float(frame_id);
        camera.radius = k0.radius + fi * d_radius;
        camera.phi = k0.phi + fi * d_phi;
        camera.z = k0.z + fi * d_z;
        camera.fov_rad = k0.fov_rad + fi * d_fov;

        frame_id += 1;
        return true;
    }


private:
    unsigned int key_id = 0;
    unsigned int frame_id = 0;

};

} // namespace viewer