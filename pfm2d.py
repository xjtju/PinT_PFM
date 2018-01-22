#!/usr/bin/env python
import sys
import os
import numpy as np
import matplotlib.pyplot as plt 
import matplotlib.cm as cm


def get_color():
    for item in ['r','g','b','c','m','y','k']:
        yield item

def main(argv):
    if(len(argv)<2):
        print 'usage   : python pfm.py input_file xpoints'
        print 'example : python pfm.py 1.txt 1000'
        return 1

    fname = argv[1]
    cnt   = int(argv[2])
    print 'input file is {0} with {1} points \n'.format(fname,cnt)

    xlen = 1.0
    ylen = 1.0
    posx  = np.zeros(cnt)
    posy  = np.zeros(cnt)
    for i in range(cnt):
        posx[i] = i*xlen/cnt
        posy[i] = i*ylen/cnt

    labels = ['0.001', '0.01', '0.02', '0.1']
    color = get_color()
    ind = 0
    tmp = np.loadtxt(fname);
    grid = tmp.reshape(cnt, cnt) 
    
    plt.imshow(grid, interpolation='nearest')
    plt.colorbar()
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title(fname)
    #axes = plt.gca()
    #axes.set_ylim([-0.1,1.1])
    #axes.set_xlim([-0.02, xlen+0.02])
    #P.savefig('pfm.eps',bbox_inches='tight')
    plt.show()

if __name__ == '__main__':
    main(sys.argv)
