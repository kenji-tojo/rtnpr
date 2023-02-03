import numpy as np
import os, sys
from typing import Tuple


# an obj loader that only loads a triangle mesh
def read_obj(path: str) -> Tuple[np.ndarray, np.ndarray]:
    if not os.path.exists(path):
        print(f'error: file {path} does not exist', file=sys.stderr)
        assert False
    
    if os.path.basename(path)[-4:] != '.obj':
        print(f'error: not an obj file', file=sys.stderr)
        assert False


    mesh = {'v': [], 'f': []}
    with open(path, 'r') as f:
        for line in f.readlines():
            if line.startswith('#'):
                continue
            
            line = line.split(' ')

            if len(line) == 0:
                continue

            if not line[0] in ['v', 'f']:
                continue

            if len(line) != 4:
                print('error: model is not a triangle mesh', file=sys.stderr)
                assert False
            
            k, v1, v2, v3 = line
            mesh[k].append([float(v1), float(v2), float(v3)])
    
    V = np.array(mesh['v'], dtype=np.float32)
    F = np.array(mesh['f'], dtype=np.int32)
    F -= 1  # index starts from 1
    print(f'loaded mesh with {len(V)} vertices and {len(F)} faces')
    return V, F


if __name__ == '__main__':
    read_obj('./assets/bunny_309_faces.obj')