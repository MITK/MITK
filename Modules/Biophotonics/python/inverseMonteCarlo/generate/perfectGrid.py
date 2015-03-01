import time
import itertools
import datetime

import numpy as np

from helper import monteCarloHelper as mch
from setup  import simulation

"""
generating:
_____
1.
    All the monte carlo simulations
2.
    Reflectance over wavelengths for the given parameter ranges (assuming a
    infinitly wide, uniform light source)

TODO:
_____

"""

def perfectGrid(generatedFilename):

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

    BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb = simulation.perfect()

    reflectances = np.zeros((nrSamples, len(wavelengths)))

    print('start simulations...')


    #%% start program logic
    start = time.time()

    currentSimulation = 0

    params = itertools.product(BVFs, Vss, ds, SaO2s, rs)
    paramsList = list(itertools.product(BVFs, Vss, ds, SaO2s, rs))

    for BVF, Vs, d, SaO2, r in params:

        print('starting simulation ' + str(currentSimulation) + ' of ' + str(nrSamples))
        for idx, wavelength in enumerate(wavelengths):

            # here the magic happens: run monte carlo simulation for tissue parameters
            # and specific wavelength
            reflectanceValue = mch.runOneSimulation(
                wavelength, eHbO2, eHb,
                infile, outfolderMC, gpumcmlDirectory, gpumcmlExecutable,
                BVF, Vs, d,
                r, SaO2,
                Fwhm = FWHM, nrPhotons=photons)

            print((BVF, Vs, d, SaO2, r, wavelength))

            # here, summarize result from wavelength in reflectance spectrum
            reflectances[currentSimulation, idx] = reflectanceValue

        currentSimulation += 1




    infile.close()

    # save the reflectance results!
    now = datetime.datetime.now().strftime("%Y%B%d%I:%M%p")
    np.save(outfolderRS + now + generatedFilename + "reflectances" + str(photons) + "photons", reflectances)
    np.save(outfolderRS + now + generatedFilename + "parameters", parameters)

    end = time.time()
    print "total time for generating perfect data on grid: " + str((end - start))