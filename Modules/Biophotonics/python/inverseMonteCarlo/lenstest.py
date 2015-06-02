# -*- coding: utf-8 -*-
"""
Created on Sun Mar  1 15:24:48 2015

@author: wirkert
"""

import numpy as np
import matplotlib.pyplot as plt

import helper.mieMonteCarlo as mmc
import helper.monteCarloHelper as mch
from setup import simulation
from csvImageReader import csvImageReader

plt.close('all')

def plotOne(liver, noobj, flatf, testsegmentation, title):

    si = sorted(range(len(simulation.wavelengths)), key=lambda k: simulation.wavelengths[k])

    #%%
    liverMean = np.mean(liver[np.nonzero(testsegmentation)[0], :], axis = 0)
    noobjMean = np.mean(noobj[np.nonzero(testsegmentation)[0], :], axis = 0)
    flatfMean = np.mean(flatf[np.nonzero(testsegmentation)[0], :], axis = 0)

    plt.figure()
    plt.plot(simulation.wavelengths[si], liverMean[si], 'r-o')
    plt.plot(simulation.wavelengths[si], noobjMean[si], 'g-o')
    plt.plot(simulation.wavelengths[si], flatfMean[si], 'b-o')

    plt.title(title)
    plt.legend(["liver", "no object", "flatfield"])
    plt.xlabel('wavelength')
    plt.ylabel('reflectance')



shape, testsegmentation = csvImageReader("data/output/liver_28_testsegmentation")


#%%
shape, liver28 = csvImageReader("data/output/liver_28")
shape, noobj28 = csvImageReader("data/output/NoObject_28")
shape, flatf28 = csvImageReader("data/output/flatfield_28")


plotOne(liver28, noobj28, flatf28, testsegmentation, "f2.8")


#%%
shape, liver22 = csvImageReader("data/output/liver_22")
shape, noobj22 = csvImageReader("data/output/NoObject_22")
shape, flatf22 = csvImageReader("data/output/flatfield_22")



plotOne(liver22, noobj22, flatf22, testsegmentation, "f22")



#%%

shape, liver8 = csvImageReader("data/output/liver_8")
shape, noobj8 = csvImageReader("data/output/NoObject_8")
shape, flatf8 = csvImageReader("data/output/flatfield_8")

#%%

plotOne(liver8, noobj8, flatf8, testsegmentation, "f8")