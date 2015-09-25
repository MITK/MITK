'''
Created on Sep 22, 2015

@author: wirkert
'''

import math

import numpy as np
import matplotlib.pyplot as plt

from mc.usuag import UsgMie, UsgJacques

if __name__ == '__main__':
    wavelengths = np.arange(350, 1350, 10) * 10 ** -9

    # set up plots
    f, axarr = plt.subplots(4, 1)
    usplt = axarr[0]
    usplt.grid()
    usplt.set_xlabel("wavelength [nm]")
    usplt.set_ylabel("us [cm-1]")
    usplt.set_yscale('log')
    usplt.set_title("scattering coefficient")
    usplt.set_xticks(np.arange(450, 720, 20))
    gplt = axarr[1]
    gplt.grid()
    gplt.set_xlabel("wavelength [nm]")
    gplt.set_ylabel("g")
    gplt.set_title("anisotropy factor")
    usrplt = axarr[2]
    usrplt.grid()
    usrplt.set_xlabel("wavelength [nm]")
    usrplt.set_ylabel("us' [cm-1]")
    usrplt.set_title("reduced scattering coefficient")
    usrplt.set_xticks(np.arange(450, 720, 20))
    aniplt = axarr[3]
    aniplt.grid()
    aniplt.set_xlabel("x = ka = size parameter")
    aniplt.set_ylabel("g")
    aniplt.set_xscale('log')
    aniplt.set_title("anisotropy")
    # set up simulation
    usg = UsgJacques()
#     usg.n_medium = 1.33
#     usg.n_particle = 1.40

    a_mie_range = np.linspace(5., 20., 4)  # 100
    a_ray_1 = np.zeros_like(a_mie_range)
    a_ray_range = np.linspace(5., 60., 7)  # * 100
    a_mie_2 = np.ones_like(a_ray_range)  # * 20. * 100
    plt_range = np.concatenate([zip(a_mie_range, a_ray_1),
                               zip(a_mie_2, a_ray_range)])
    for i, p in enumerate(plt_range):
        # set and calculate values
        usg.a_mie = p[0]
        usg.a_ray = p[1]
        us = [usg(w)[0] for w in wavelengths]
        g = [usg(w)[1] for w in wavelengths]
        # plot stuff
        # from blue to red: the color of the plotted curves
        plt_color = (1. / float(len(plt_range)) * i,
               0.,
               1. - (1. / float(len(plt_range)) * i))
        # plot scattering coefficient
        usplt.plot(wavelengths * 10 ** 9, np.array(us) / 100.,
                    '-o',
                    color=plt_color)
        # plot anisotropy factor
        gplt.plot(wavelengths * 10 ** 9, g,
                  '-o',
                  color=plt_color)
        # plot reduced scattering coefficient
        usrplt.plot(wavelengths * 10 ** 9,
                    np.array(us) * (1.0 - np.array(g)) / 100.,
                    '-o',
                    color=plt_color)
        # aniplt.plot(2. * math.pi * usg.r / wavelengths * usg.n_medium, g)
    plt.show()
