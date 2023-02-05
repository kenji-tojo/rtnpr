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
        "rt.spp": 128,
        "rt.spp_frame": 1,
        "rt.depth": 4,

        "flr.linewidth": 1.5,
        "flr.enable": True,
        "flr.line_only": False,
        "flr.wireframe": True,
        "flr.n_aux": 4,

        "tone.map_shading": True
    }

    import rtnpr as m

    img, opts = m.run_gui(V,F,opts)
    img = np.array(img)
    assert img.dtype == np.float32
    print(opts)

    if img.size > 0:
        img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
        os.makedirs('./output', exist_ok=True)
        img.save('./output/screenshot.png')
