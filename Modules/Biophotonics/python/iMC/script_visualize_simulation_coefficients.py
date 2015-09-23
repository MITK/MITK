'''
Created on Sep 22, 2015

@author: wirkert
'''

import math

import numpy as np
import matplotlib.pyplot as plt

from mc.usuag import UsG

if __name__ == '__main__':
    # set up plots
    f, axarr = plt.subplots(1, 4)
    usplt = axarr[0]
    usplt.grid()
    usplt.set_xlabel("wavelengths [nm]")
    usplt.set_ylabel("us [cm-1]")
    usplt.set_title("scattering coefficient")
    gplt = axarr[1]
    gplt.grid()
    gplt.set_xlabel("wavelengths [nm]")
    gplt.set_ylabel("g")
    gplt.set_title("anisotropy factor")
    usrplt = axarr[2]
    usrplt.grid()
    usrplt.set_xlabel("wavelengths [nm]")
    usrplt.set_ylabel("us' [cm-1]")
    usrplt.set_title("reduced scattering coefficient")
    aniplt = axarr[3]
    aniplt.grid()
    aniplt.set_xlabel("x = ka = size parameter")
    aniplt.set_ylabel("g")
    aniplt.set_xscale('log')
    aniplt.set_title("anisotropy")
    # set up simulation
    usg = UsG()
    usg.dsp = 0.04
#     usg.n_medium = 1.33
#     usg.n_particle = 1.40
    wavelengths = np.arange(400, 700, 10) * 10 ** -9

    plt_range = np.array([0.4 / 2. * 10 ** -6])
    # np.linspace(2., 3., 10) * 10 ** -6
    # np.array([579. / 2. * 10 ** -9])
    # np.linspace(0.1, 0.74, 10) * 10 ** -6
    for i, d in enumerate(plt_range):
        # set and calculate values
        usg.r = d / 2.
        us = [usg(w)[0] for w in wavelengths]
        g = [usg(w)[1] for w in wavelengths]
        g = np.array(g) / np.array(g) * 0.92
        # plot stuff
        # from blue to red: the color of the plotted curves
        plt_color = (1. / float(len(plt_range)) * i,
               0.,
               1. - (1. / float(len(plt_range)) * i))
        # plot scattering coefficient
        usplt.plot(wavelengths * 10 ** 9, np.array(us) / 100., color=plt_color)
        # plot anisotropy factor
        gplt.plot(wavelengths * 10 ** 9, g, color=plt_color)
        # plot reduced scattering coefficient
        usrplt.plot(wavelengths * 10 ** 9, np.array(us) * (1.0 - np.array(g)) / 100.,
                    color=plt_color)
        aniplt.plot(2. * math.pi * usg.r / wavelengths * usg.n_medium, g)
    plt.show()
