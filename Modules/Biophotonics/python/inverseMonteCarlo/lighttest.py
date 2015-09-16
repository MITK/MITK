# -*- coding: utf-8 -*-
"""
Created on Wed Mar  4 15:44:11 2015

@author: wirkert
"""

from csvImageReader import csvImageReader
import numpy as np
import matplotlib.pyplot as plt
from   setup import simulation

#%% load

shape, links = csvImageReader("data/output/CC2_links")
shape, flatfseglinks = csvImageReader("data/output/CC2_links_flatfieldsegmentation")

shape, rechts= csvImageReader("data/output/CC2_rechts")
shape, flatfsegrechts = csvImageReader("data/output/CC2_rechts_flatfieldsegmentation")

#%% process

wav = simulation.wavelengths
si = sorted(range(len(simulation.wavelengths)), key=lambda k: simulation.wavelengths[k])



linksMean = np.mean(links[np.nonzero(flatfseglinks)[0], :], axis = 0)
rechtsMean = np.mean(rechts[np.nonzero(flatfsegrechts)[0], :], axis = 0)


plt.close('all')

plt.figure()
plt.plot(wav[si], linksMean[si])
plt.plot(wav[si], rechtsMean[si]*1.905)
plt.legend(["light source left", "light source right"])



#%% load

shape, flatfield = csvImageReader("data/output/ff")
shape, segLight = csvImageReader("data/output/ff_testsegmentation")
shape, segDark = csvImageReader("data/output/ff_trainsegmentation")

#%% process

segLightMean = np.mean(flatfield[np.nonzero(segLight)[0], :], axis = 0)
segDarkMean = np.mean(flatfield[np.nonzero(segDark)[0], :], axis = 0)



plt.close('all')

plt.figure()
plt.plot(wav[si], segLightMean[si])
plt.plot(wav[si], segDarkMean[si]*1.53)
plt.legend(["light source light", "light source dark"])
