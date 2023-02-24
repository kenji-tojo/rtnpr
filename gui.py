import numpy as np
from PIL import Image
import os, shutil

from read_obj import *


COMMAND_NONE             = 0
COMMAND_RENDER_IMAGE     = 1
COMMAND_RENDER_ANIMATION = 2


BRDF_PHONG      = 0
BRDF_LAMBERTIAN = 1
BRDF_GLOSSY     = 2
BRDF_SPECULAR   = 3


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    args = parser.parse_args()

    V,F = read_obj(args.path)

    params = {
        'opts:rt.spp': 32,
        'opts:rt.spp_frame': 1,
        'opts:rt.depth': 4,

        'opts:flr.intensity': 1e3,
        'opts:flr.width': 1.5,
        'opts:flr.enable': True,
        'opts:flr.line_only': False,
        'opts:flr.wireframe': False,
        'opts:flr.n_aux': 4,

        'opts:tone.mapper.theme_id': 0,
        'opts:tone.map_lines': False,

        'camera:position.x()': 0.,
        'camera:position.y()': -135.,
        'camera:position.z()': 80.,
        'camera:fov_rad': np.pi/12.,

        'scene:light->position.x()': 0.,
        'scene:light->position.y()': -1.,
        'scene:light->position.z()': 1.,

        'scene:plane().mat_id': BRDF_LAMBERTIAN,
        'scene:plane().checkerboard': True,
        'scene:plane().check_res': 10
    }

    import rtnpr as m

    params = m.run_gui(V,F,params)

    if params['renderer_params:cmd'] == COMMAND_RENDER_IMAGE:
        params['opts:rt.spp_frame'] = 16
        img, params = m.run_headless(V,F,params)
        assert img.size > 1
        assert img.dtype == np.float32
        img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
        os.makedirs('./output', exist_ok=True)
        img.save('./output/screenshot.png')

    elif params['renderer_params:cmd'] == COMMAND_RENDER_ANIMATION:
        ANIMATION_OUT_DIR = './output/animation'
        if os.path.exists(ANIMATION_OUT_DIR):
            shutil.rmtree(ANIMATION_OUT_DIR)
        os.makedirs(ANIMATION_OUT_DIR)

        while params['renderer_params:cmd'] == COMMAND_RENDER_ANIMATION:
            frame_id = params['renderer_params:anim.frame_id']
            frames = params['renderer_params:anim.frames']
            params['opts:rt.spp_frame'] = 16
            img, params = m.run_headless(V,F,params)
            assert img.size > 1
            assert img.dtype == np.float32
            img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
            img.save(os.path.join(ANIMATION_OUT_DIR, f'{frame_id:03d}.png'))
        
        framerate = 30
        os.system(f'ffmpeg -framerate {framerate} -i {ANIMATION_OUT_DIR}/%03d.png {ANIMATION_OUT_DIR}/video.mp4')
