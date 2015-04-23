# -*- coding: utf-8 -*-
"""
Created on Sun Feb 01 13:52:23 2015
Some helper methods

@author: Seb
"""

import math
import numpy as np
import mieMonteCarlo
import subprocess
import os
from setup import systemPaths

import contextlib

@contextlib.contextmanager
def cd(newPath):
    savedPath = os.getcwd()
    os.chdir(newPath)
    yield
    os.chdir(savedPath)


def createSimulationFile(infile, replacements, simulationFileName):
    infile.seek(0);
    simulationFile = open(simulationFileName, 'w')

    for line in infile:
        for src, target in replacements.iteritems():
            line = line.replace(src, target)
        simulationFile.write(line)

    simulationFile.close()

def calcUa(BVF, cHb, SaO2, eHbO2, eHb, wavelength):
    # determine ua [1/m] as combination of extinction coefficients.
    # For more on this equation, please refer to
    # http://omlc.org/spectra/hemoglobin/
    return math.log(10) * cHb * \
        (SaO2 * eHbO2(wavelength) +  (1-SaO2) * eHb(wavelength)) \
        / 64500 * BVF

def calcUsMuscle(wavelength):
    return 168.52 * (wavelength * 10**9)**-0.332 / (1 - 0.96) * 100

def getReflectanceFromFile(fileName):
    '''
    extract reflectance from a specific monte carlo output file.

    Attention: this is kept very simple and assumes that the reflectance output
    starts at a specific line and keeps till eof. This might be subject to change,
    e.g., when you alter the numbers of layers the line number there reflectance
    values start will change.
    Error handling etc. needed.
    '''

    with open(fileName) as myFile:
        for num, line in enumerate(myFile, 1):
            if "Rd_r #Rd[0], [1],..Rd[nr-1]. [1/cm2]" in line:
                break

    #print 'found at line:', num

    reflectances = np.loadtxt(fileName, skiprows=num)

    return sum(reflectances)


from setup import simulation

def normalizeIntegral(data):
    # normalize data
    # first sort by wavelength:
    ourWavelengths = simulation.wavelengths
    #ourWavelengths = np.delete(simulation.getWavelengths(), [2,7])

    sortedIndices = sorted(range(len(ourWavelengths)), key=lambda k: ourWavelengths[k])

    norms = np.trapz(data[:,sortedIndices], ourWavelengths[sortedIndices], axis=1)
    return data / norms[:,None]

def normalizeSum(data):
    return data/data.sum(axis=1)[:,None]

def normalizeMean(data):
    return data - data.mean(axis=1)[:,None]

iqBand = 2

def normalizeImageQuotient(data):


    # use line iqBand as image quotient
    normData = data / data[:,iqBand][:,None]
    # discard it
    normData = np.delete(normData, iqBand, axis=1)

    #normData = normalizeMean(normData)

    return normData
    #return normalizeIntegral(data)

    #return normalizeMean(firstNorm)

def removeIqWavelength(wavelenghts):
    #return wavelenghts
    return np.delete(wavelenghts, iqBand)

def runOneSimulation(wavelength, eHbO2, eHb,
                     infile, simulationOutputFolderName, gpumcmlDirectory, gpumcmlExecutable,
                     BVF=0.1, Vs=0.4, d=500 * 10**-6,
                     r=0.4 * 10**-6, SaO2=0.7, cHb=120,
                     submucosa_BVF=0.1, submucosa_Vs=0.3, submucosa_r = 2*10**-6, submucosa_SaO2=0.7,
                     Fwhm = 20 * 10**-9, nrPhotons = 10**6,
                     ):
    """
    Args:
    _____
    cHb:
        concentration of haemoglobin per unit volume of blood in colon [g/L],
        taken from: "Modelling and validation of spectral reflectance for the colon"
    """
    outputFilename = \
            simulationOutputFolderName + \
            "BVF" + repr(BVF) + \
            "Vs" + repr(Vs) + \
            "d" + repr(d) + \
            "SaO2" + repr(SaO2) + \
            "r" + repr(r) + \
            "photons" + repr(nrPhotons) + \
            "wavelength" + repr(wavelength) + \
            ".mco"

    usg = mieMonteCarlo.mieMonteCarlo_FWHM(wavelength, r, Vs, FWHM = Fwhm)

    ua  = calcUa(BVF, cHb, SaO2, eHbO2, eHb, wavelength)
    # calculate ua, us for submucosa, values taken from
    # "Model based inversion for deriving maps of
    # histological parameters characteristic of cancer
    # from ex-vivo multispectral images of the colon"
    ua_sm  = calcUa(submucosa_BVF, cHb, submucosa_SaO2, eHbO2, eHb, wavelength)
    usg_sm = mieMonteCarlo.mieMonteCarlo_FWHM(wavelength,submucosa_r, submucosa_Vs, FWHM = Fwhm)

    # now create us and ua for muscle layer according to
    # "Modelling and validation of spectral reflectance for the colon"
    us_mus = calcUsMuscle(wavelength)
    A      = 1.7923385088285804 # calculated to retrieve an absorption of 11.2 cm-1
    # at 515nm
    ua_mus = calcUa(A*0.1, 120, 0.7, eHbO2, eHb, wavelength)


    # factors are due to the program expecting [cm] instead of [m]
    replacements = {'muc_c_ua':str(ua / 100), 'muc_c_us':str(usg['us'] / 100),
        'muc_c_g':str(usg['g']), 'muc_c_d':str(d * 100),
        'sm_c_ua':str(ua_sm / 100), 'sm_c_us':str(usg_sm['us'] / 100),
        'sm_c_g':str(usg_sm['g']),
        'mus_c_ua':str(ua_mus / 100), 'mus_c_us':str(us_mus / 100),
        'outputFilename':outputFilename,
        'c_ph': str(nrPhotons)}

    createSimulationFile(infile, replacements,
                         systemPaths.getOutputFolder() + "simulationFile.mci")

    args = ("./" + gpumcmlExecutable, "-A", systemPaths.getOutputFolder() + "simulationFile.mci")

    with cd(gpumcmlDirectory):
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()

    # outside the context manager we are back wherever we started.



    # this function is error prone, please refer to documentation to see
    # if problematic for you
    return getReflectanceFromFile(gpumcmlDirectory + outputFilename)


if __name__ == "__main__":

    us515 = calcUsMuscle(515 * 10**-9)
    print ("calculated us [cm-1] for muscle layer at 515nm: " + str(us515 / 100) + " expected ca. 530")
    us1064 = calcUsMuscle(1064 * 10**-9)
    print ("calculated us [cm-1] for muscle layer at 1064nm: " + str(us1064 / 100))