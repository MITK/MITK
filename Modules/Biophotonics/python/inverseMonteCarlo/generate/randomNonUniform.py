# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 15:15:23 2015

@author: wirkert
"""

# -*- coding: utf-8 -*-
"""
Created on Fri Feb  6 16:14:57 2015

@author: wirkert
"""

import time
import datetime
import random

import numpy as np

from helper import monteCarloHelper as mch
from setup import simulation
from setup import systemPaths

def drawFromNormal(parameterVector):
    """
    helper function to draw a sample from a normal distribution clipped by
    the boarders of the parameter range
    """
    # draw sample from some normal distribution
    sample = random.gauss(np.mean(parameterVector), (max(parameterVector) - min (parameterVector)) / 10.0)
    # clip it to be in the allowed range
    sample = np.clip(sample, min(parameterVector), max(parameterVector))
    return sample



def randomNonUniform(generatedFilename):


    infileString, outfolderMC, outfolderRS, gpumcmlDirectory, gpumcmlExecutable = systemPaths.initPaths()
    infile = open(infileString)


    BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations = simulation.noisy()

    reflectances  = np.zeros((nrSimulations, len(wavelengths)))
    parameters    = np.zeros((nrSimulations, 8))

    print('start simulations...')

    #%% start program logic
    start = time.time()


    for i in range(nrSimulations):

        print('starting simulation ' + str(i) + ' of ' + str(nrSimulations))

        BVF = drawFromNormal(BVFs)
        Vs  = drawFromNormal(Vss)
        d   = drawFromNormal(ds)
        SaO2= drawFromNormal(SaO2s)
        r   = drawFromNormal(rs)

        min_sm_BVF = max(min(BVFs), 0.03)
        sm_BVF = drawFromNormal([min_sm_BVF, max(BVFs)])
        sm_Vs  = drawFromNormal(Vss)
        sm_SaO2= drawFromNormal(SaO2s)

        parameters[i,:] = np.array([BVF, Vs, d, SaO2, r, sm_BVF, sm_Vs, sm_SaO2])


        for j, wavelength in enumerate(wavelengths):


            reflectanceValue = mch.runOneSimulation(
                wavelength, eHbO2, eHb,
                infile, outfolderMC, gpumcmlDirectory, gpumcmlExecutable,
                BVF, Vs, d,
                r, SaO2,
                Fwhm = FWHM, nrPhotons=photons)


            print((BVF, Vs, d, SaO2, r, wavelength))

            # here, summarize result from wavelength in reflectance spectrum
            reflectances[i, j] = reflectanceValue




    infile.close()

    # save the reflectance results!
    now = datetime.datetime.now().strftime("%Y%B%d%I:%M%p")
    np.save(outfolderRS + now + generatedFilename + "reflectances" + str(photons) + "photons", reflectances)
    np.save(outfolderRS + now + generatedFilename  + str(nrSimulations) + "parameters", parameters)

    end = time.time()
    print "total time for generating random non uniform data: " + str((end - start))