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

def drawFromNormal(parameterVector):
    """
    helper function to draw a sample from a normal distribution clipped by
    the boarders of the parameter range
    """
    # draw sample from some normal distribution
    sample = random.gauss(np.mean(parameterVector), (max(parameterVector) - min (parameterVector)) /20.0)
    # clip it to be in the allowed range
    sample = np.clip(sample, min(parameterVector), max(parameterVector))
    return sample



def randomNonUniform(generatedFilename):
    # the input file without the run specific parameters for ua, us and d:
    infileString = 'data/colonTemplate.mci'
    infile       = open(infileString)
    # the output folder for the mc simulations
    outfolderMC = 'outputMC/'
    # the output folder for the reflectance spectra
    outfolderRS = 'outputRS/'


    # the input file without the run specific parameters for ua, us and d:
    infileString = 'data/colonTemplate.mci'
    infile       = open(infileString)
    # the output folder for the mc simulations
    # attention: this is relative to your gpumcml path!
    outfolderMC ='outputMC/'
    # the output folder for the reflectance spectra
    outfolderRS = 'data/output/'
    gpumcmlDirectory = '/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/'
    gpumcmlExecutable = 'gpumcml.sm_20'


    BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations = simulation.perfect()

    reflectances  = np.zeros((nrSimulations, len(wavelengths)))
    parameters    = np.zeros((nrSimulations, 5))

    print('start simulations...')

    #%% start program logic
    start = time.time()


    for i in range(nrSimulations):

        print('starting simulation ' + str(i) + ' of ' + str(nrSimulations))

        BVF = drawFromNormal(BVFs)
        Vs  = drawFromNormal(Vss)
        d   = drawFromNormal(ds)
        r   = drawFromNormal(rs)
        SaO2= drawFromNormal(SaO2s)

        parameters[i,:] = np.array([BVF, Vs, d, r, SaO2])


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
    np.save(outfolderRS + now + generatedFilename + "parameters", parameters)

    end = time.time()
    print "total time for generating random non uniform data: " + str((end - start))