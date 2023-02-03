import numpy as np
from read_obj import *


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to the input obj file')
    args = parser.parse_args()

    V,F = read_obj(args.path)

    import rtnpr as m
    m.run_gui(V,F)
