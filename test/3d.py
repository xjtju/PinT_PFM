#!/usr/bin/env python
import sys
import os
import h5py
import math
import numpy as np
import matplotlib.pyplot as plt 
import matplotlib.cm as cm
from mpl_toolkits.mplot3d import Axes3D

def get_color():
    for item in ['r','g','b','c','m','y','k']:
        yield item

def main(argv):
    if(len(argv)<4):
        print 'usage   : python pfm.py input_file label side_length'
        print 'example : python pfm.py result.h5 bd4 32'
        return 1

    fname = argv[1]
    lname = argv[2]
    sidelen =  int(argv[3]) 
    print 'input file is {0} \n'.format(fname)

    xlen = 1.0
    ylen = 1.0
    
    labels = ['0.001', '0.01', '0.02', '0.1']
    color = get_color()
    ind = 0
    pf = h5py.File(fname, 'r')
    ds1 = pf.get('solution')
    ds2 = pf.get('coords')
    (totalcount, dims) = ds2.shape
    print 'coords shape is ( {0}^3, {1} )\n'.format(sidelen, dims)
    if(sidelen*sidelen*sidelen != totalcount):
        print 'error: totalcount must be equal to sidelen^3 {0}!={1}^3 \n'.format(totalcount, sidelen)
    posx = ds2[..., 0]
    posy = ds2[..., 1]
    posz = ds2[..., 2]
    
    # calculate the coordinates of a crosssection of Z direction
    iz = np.arange(sidelen*sidelen) * sidelen + sidelen/2
    iy = np.arange(sidelen*sidelen) + sidelen 
    ix = np.arange(sidelen*sidelen) 

    grid = ds1[...]
    sgrid = grid[iz]  

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    p = ax.scatter3D(posx, posy, posz, c=grid, s=1, edgecolor='none')
    #p = ax.scatter3D(posx, posy, posz, c=grid, s=1, edgecolor='none', cmap=cm.binary)
    # add a cross section of Z direction for clearly seeing the figure  
    plt.scatter(posx[ix], posy[iy], c=sgrid, edgecolor='none' )
    #plt.colorbar()
    fig.colorbar(p)
    ax = plt.gca()
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    #ax.set_zlabel('Z')
    ax.set_xlim(0, 1.0)
    ax.set_ylim(0, 1.0)
    #ax.set_zlim(0, 1.0) 
    plt.title(lname)

    #axes = plt.gca()
    #axes.set_ylim([-0.1,1.1])
    #axes.set_xlim([-0.02, xlen+0.02])
    #p.savefig('pfm3d.png',bbox_inches='tight')
    plt.show()

if __name__ == '__main__':
    main(sys.argv)
