import numpy as np
from PIL import Image
import imageio

import os, shutil

from read_obj import *


COMMAND_NONE             = 0
COMMAND_RENDER_IMAGE     = 1
COMMAND_RENDER_ANIMATION = 2


BRDF_PHONG      = 0
BRDF_LAMBERTIAN = 1
BRDF_GLOSSY     = 2
BRDF_SPECULAR   = 3

TONE_REINHARD = 0
TONE_LINEAR   = 1
TONE_RAW      = 2

UINT16_MAX = np.iinfo(np.uint16).max


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    args = parser.parse_args()

    V,F = read_obj(args.path)

    import rtnpr as m

    camera = m.Camera()
    light  = m.Light(m.Light.SoftDirectional)
    mesh   = m.TriMesh(V,F)
    scene  = m.Scene()

    camera.set_position(0., -135., 80.)
    camera.set_fov(15.)

    light.set_position(0., -1., 1.)

    mesh.visible = True
    mesh.scale = 1.
    mesh.shift_z = 0.

    scene.set_camera(camera)
    scene.set_light(light)
    scene.add_mesh(mesh)

    scene.plane_mat_id = BRDF_LAMBERTIAN
    scene.plane_checkerboard = True
    scene.plane_check_res = 10
    scene.plane_albedo = .2
    scene.plane_visible = True

    scene.phong_kd = .05
    scene.phong_power = 30
    scene.phong_albedo = .7


    options = m.Options()
    options.rt_spp = 32
    options.rt_spp_frame = 1
    options.rt_depth = 4
    options.flr_intensity = 1e3
    options.flr_width = 1.5
    options.flr_enable = True
    options.flr_line_only = False
    options.flr_wireframe = False
    options.flr_n_aux = 4
    options.tone_mode = TONE_REINHARD
    options.tone_theme_id = 0
    options.tone_map_lines = False


    m.show(scene, options)


    if scene.get_command() == COMMAND_RENDER_IMAGE:
        options.rt_spp_frame = 16
        img = m.render(scene, options)
        assert img.size > 1
        assert img.dtype == np.float32

        os.makedirs('./output', exist_ok=True)

        if options.tone_mode == TONE_RAW:
            max_pixel = np.max(img)
            print('max pixel:', max_pixel)
            img = np.round(5e-1*UINT16_MAX*img).clip(0,UINT16_MAX).astype(np.uint16)
            imageio.imsave('./output/screenshot.tiff', img)
        
        else:
            img = Image.fromarray(np.round(img*255.).clip(0,255).astype(np.uint8))
            img.save('./output/screenshot.png')

    elif scene.get_command() == COMMAND_RENDER_ANIMATION:
        ANIMATION_OUT_DIR = './output/animation'
        if os.path.exists(ANIMATION_OUT_DIR):
            shutil.rmtree(ANIMATION_OUT_DIR)
        os.makedirs(ANIMATION_OUT_DIR)

        while scene.get_command() == COMMAND_RENDER_ANIMATION:
            frame_id = scene.get_frame_id()
            frames = scene.get_frames()
            options.rt_spp_frame = 16
            img = m.render(scene, options)
            assert img.size > 1
            assert img.dtype == np.float32
            img = Image.fromarray((img*255.+.5).clip(0,255).astype(np.uint8))
            img.save(os.path.join(ANIMATION_OUT_DIR, f'{frame_id:03d}.png'))
        
        framerate = 30
        os.system(f'ffmpeg -framerate {framerate} -i {ANIMATION_OUT_DIR}/%03d.png {ANIMATION_OUT_DIR}/video.mp4')
