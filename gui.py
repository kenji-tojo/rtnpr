import numpy as np
from PIL import Image
import os

from read_obj import *


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    args = parser.parse_args()

    V,F = read_obj(args.path)

    opts = {
        'opts:rt.spp': 128,
        'opts:rt.spp_frame': 1,
        'opts:rt.depth': 4,
        'opts:flr.linewidth': 1.5,
        'opts:flr.enable': True,
        'opts:flr.line_only': False,
        'opts:flr.wireframe': True,
        'opts:flr.n_aux': 4,
        'opts:tone.map_shading': True,
        'camera:position.x()': 0.,
        'camera:position.y()': -4.,
        'camera:position.z()': 2.5,
        'camera:fov_rad': np.pi/12.
    }

    import rtnpr as m

    opts = m.run_gui(V,F,opts)

    if opts['run_headless']:
        img = m.run_headless(V,F,opts)
        assert img.size > 1
        assert img.dtype == np.float32
        img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
        os.makedirs('./output', exist_ok=True)
        img.save('./output/screenshot.png')
