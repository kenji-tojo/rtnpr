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


def create_sundirs(n_frames: int) -> np.ndarray:
    b1 = np.array([1.,0.,0.])
    b2 = np.array([0.,-1.,1.])
    b2 /= np.linalg.norm(b2)

    phi = np.linspace(0.,1.,9) * np.pi
    start = phi[1]
    end = phi[-2]
    phi = np.linspace(start,end,n_frames)

    dirs = np.cos(phi)[:,None] * b1 + np.sin(phi)[:,None] * b2
    return dirs


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    parser.add_argument('--diff', action='store_true', help='rendering diff image')
    parser.add_argument('--scale', type=float, default=1., help='scale applied to the model')
    parser.add_argument('--shift_z', type=float, default=0., help='vertical shift applied to the model')
    parser.add_argument('--fov', type=float, default=60., help='fov of a camera')
    parser.add_argument('--spp', type=int, default=32, help='number of samples per pixel')

    args = parser.parse_args()

    V,F = read_obj(args.path)

    import rtnpr as m

    camera = m.Camera()
    light  = m.Light(m.Light.SoftDirectional)
    mesh   = m.TriMesh(V,F)
    scene  = m.Scene()

    camera.set_position(0.,-180.,100.)
    camera.look_at(0.,0.,0.)
    camera.fov = args.fov

    light.set_position(0.,-1.,1.)
    light.look_at(0.,0.,0.)

    mesh.visible = True
    mesh.scale = args.scale
    mesh.shift_z = args.shift_z

    scene.set_camera(camera)
    scene.set_light(light)
    scene.add_mesh(mesh)

    scene.plane_mat_id = BRDF_LAMBERTIAN
    scene.plane_checkerboard = True
    scene.plane_check_res = 10
    scene.plane_albedo = .2
    scene.plane_visible = True
    scene.plane_transparent = False

    scene.phong_kd = .05
    scene.phong_power = 30
    scene.phong_albedo = .7


    options = m.Options()
    options.rt_spp = args.spp
    options.rt_spp_frame = 1
    options.rt_depth = 4
    options.rt_surface_normal = False
    options.flr_width = 1.5
    options.flr_enable = False
    options.flr_line_only = False
    options.flr_wireframe = False
    options.flr_n_aux = 4
    options.tone_mode = TONE_REINHARD
    options.tone_theme_id = 0


    m.show(scene, options)

    def render_image(scene, options) -> np.ndarray:
        img = np.zeros((options.img_height,options.img_width,4), dtype=np.float32)
        m.render(scene, options, img)
        return img

    if scene.get_command() == COMMAND_RENDER_IMAGE:
        options.rt_spp_frame = 32

        img = render_image(scene, options)

        os.makedirs('./output', exist_ok=True)

        if options.tone_mode == TONE_RAW:
            max_pixel = np.max(img)
            print('max pixel:', max_pixel)
            img = img[:,:,:3]
            img = np.round(1e-1*UINT16_MAX*img).clip(0,UINT16_MAX).astype(np.uint16)
            imageio.imsave('./output/screenshot.tiff', img)
        
        else:
            img = Image.fromarray(np.round(img*255.).clip(0,255).astype(np.uint8))
            img.save('./output/screenshot.png')

    elif scene.get_command() == COMMAND_RENDER_ANIMATION:
        ANIMATION_OUT_DIR = './output/animation'
        if os.path.exists(ANIMATION_OUT_DIR):
            shutil.rmtree(ANIMATION_OUT_DIR)
        os.makedirs(ANIMATION_OUT_DIR)

        n_frames = scene.get_frames()

        ldir, cpos = None, None

        if scene.is_light_animated():
            ldir = create_sundirs(n_frames)

        if scene.is_camera_animated():
            cpos = np.zeros((n_frames, 3), dtype=np.float32)
            m.create_camera_positions(scene, cpos)
    
        for frame_id in range(n_frames):
            if ldir is not None:
                d = ldir[frame_id]
                light.set_dir(d[0], d[1], d[2])

            if cpos is not None:
                p = cpos[frame_id]
                camera.set_position(p[0], p[1], p[2])
                camera.look_at(0.,0.,0.)

            options.rt_spp_frame = 32

            if args.diff:
                spp = options.rt_spp

                options.tone_mode = TONE_REINHARD
                scene.plane_transparent = True
                img = render_image(scene, options)
                scene.plane_transparent = False

                options.rt_spp = min(64, spp)

                options.tone_mode = TONE_RAW
                mesh.visible = False
                img_bg = render_image(scene, options)[:,:,:3]
                img_bg = np.mean(img_bg, axis=2)
                mesh.visible = True

                options.rt_spp = spp

                img_fg = render_image(scene, options)[:,:,:3]
                img_fg = np.mean(img_fg, axis=2)

                scale = 1.
                img_diff = np.clip(scale*(img_fg-img_bg),0.,1.)
                from matplotlib import cm
                cmap = cm.get_cmap('viridis')
                img_diff = cmap(img_diff)

                img[:,:,:3] *= img[:,:,3][:,:,None]
                img[:,:,:3] += (1.-img[:,:,3][:,:,None])*img_diff[:,:,:3]
                img[:,:,3] = 1.
            
            else:
                img = render_image(scene, options)
                
            img = Image.fromarray(np.round(img*255.).clip(0,255).astype(np.uint8))
            img.save(os.path.join(ANIMATION_OUT_DIR, f'{frame_id:03d}.png'))
        
        framerate = 30
        os.system(f'ffmpeg -framerate {framerate} -i {ANIMATION_OUT_DIR}/%03d.png {ANIMATION_OUT_DIR}/video.mp4')
