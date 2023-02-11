import numpy as np
from PIL import Image
import os

from read_obj import *


COMMAND_NONE = 0
COMMAND_RENDER_IMAGE = 1
COMMAND_RENDER_ANIMATION = 2


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    args = parser.parse_args()

    V,F = read_obj(args.path)

    params = {
        'opts:rt.spp': 128,
        'opts:rt.spp_frame': 1,
        'opts:rt.depth': 4,

        'opts:flr.linewidth': 1.5,
        'opts:flr.enable': True,
        'opts:flr.line_only': False,
        'opts:flr.wireframe': True,
        'opts:flr.n_aux': 4,

        'opts:tone.mapper.theme_id': 1,

        'camera:position.x()': 0.,
        'camera:position.y()': -135.,
        'camera:position.z()': 80.,
        'camera:fov_rad': np.pi/12.
    }

    import rtnpr as m

    params = m.run_gui(V,F,params)

    if params['command'] == COMMAND_RENDER_IMAGE:
        params['opts:rt.spp_frame'] = 16
        img, params = m.run_headless(V,F,params)
        assert img.size > 1
        assert img.dtype == np.float32
        img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
        os.makedirs('./output', exist_ok=True)
        img.save('./output/screenshot.png')
    elif params['command'] == COMMAND_RENDER_ANIMATION:
        pass
