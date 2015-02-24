# -*- coding: utf-8 -*-
"""
Created on Thu Feb 19 10:14:34 2015

@author: wirkert
"""

from setup import data

import csvImageReader

import helper.monteCarloHelper as mch
import numpy as np
import time
import Image
from sklearn.ensemble  import RandomForestRegressor
from matplotlib.pyplot import plot


#%% 0. setup helper functions.

def find_nearest(array,value):
    errors = array - value
    quadratic_errors = errors * errors
    quadratic_error  = np.mat(quadratic_errors) * np.mat(np.ones((value.shape[0], 1)))

    idx = quadratic_error.argmin()
    return array[idx]


#%% 1. first train a random forest classifier using noisy data
# in future versions of this script this step shall be replaced by loading a pretrained classifier

# the folder with the reflectance spectra
dataFolder = 'data/output/'

# load data
trainingParameters, trainingReflectances = \
    data.realImageEstimationRFTrainingData(dataFolder)

# train forest
rf = RandomForestRegressor(100, max_depth=10, n_jobs=5)
rf.fit(trainingReflectances, trainingParameters)

##% 2. load the image

imageToLoad = "sample_1"
shape, image, segmentation = csvImageReader.csvMultiSpectralImageReader("data/output/" + imageToLoad)
image        = mch.normalizeImageQuotient(image, 1)


#%% 3. estimate the parameters for the image

start = time.time()

estimatedParameters = rf.predict(image)

# set to zero if not in segmentation mask
estimatedParameters[np.where(0 == segmentation), :] = 0


end = time.time()
print "time necessary to estimate parameters for image [s]: " + str((end - start))


#%% 4. save parametric map

for i in np.arange(0,estimatedParameters.shape[1]):
    parameterImage_i = np.reshape(estimatedParameters[:,i], shape)
    im = Image.fromarray(parameterImage_i)
    im.save(dataFolder + imageToLoad + "_parameterImage_" + str(i) + ".tiff")


#%% 5. evaluate data

# for this, create monte carlo simulation for each
# parameter estimate. The resulted reflectance estimate can then be compared to
# the measured reflectance.

from setup import systemPaths
from setup import simulation

import helper.monteCarloHelper as mch

infileString, outfolderMC, outfolderRS, gpumcmlDirectory, gpumcmlExecutable = systemPaths.initPaths()
infile = open(infileString)

BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations = simulation.perfect()

# the estimated parameters within the segmentation
estimatedParametersOnlySegmented    = estimatedParameters[np.nonzero(segmentation)[0], :]
# the image reflectances from which these parameters where estimated
inputReflectancesOnlySegmented      = image[np.nonzero(segmentation)[0], :]
# placeholder for the reflectance computed from MC with the estimated parameters
irShape = inputReflectancesOnlySegmented.shape
reflectancesFromEstimatedParameters = np.zeros((irShape[0], irShape[1]+1))
# +1 due to the non discarded image quotient

for i, (BVF, Vs, d, SaO2) in enumerate(estimatedParametersOnlySegmented):


    print('starting simulation ' + str(i) + ' of ' + str(estimatedParametersOnlySegmented.shape[0]))

    for j, wavelength in enumerate(wavelengths):

        reflectanceValue = mch.runOneSimulation(
            wavelength, eHbO2, eHb,
            infile, outfolderMC, gpumcmlDirectory, gpumcmlExecutable,
            BVF, Vs, d,
            np.median(rs), SaO2,
            Fwhm = FWHM, nrPhotons=photons)


        print((BVF, Vs, d, SaO2, wavelength))
        reflectancesFromEstimatedParameters[i, j] = reflectanceValue
    # for now only do 100 reflectances for testing
    if i>100:
        break

# correct these reflectances by image quotient
reflectancesFromEstimatedParameters = mch.normalizeImageQuotient(reflectancesFromEstimatedParameters, iqBand=1)

#%% plot data for nicer inspection
plot_i = 99
nearestTraining = find_nearest(trainingReflectances, inputReflectancesOnlySegmented[0,:])
#plot(nearestTraining);
plot(reflectancesFromEstimatedParameters[plot_i,:])
plot(inputReflectancesOnlySegmented[plot_i,:])


