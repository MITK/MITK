# -*- coding: utf-8 -*-
"""
Created on Thu Feb 19 10:14:34 2015

@author: wirkert
"""


from randomForest import randomForest

from calculateWeightsForDomainAdaption import calculateWeights

import random
import numpy as np
import time

import matplotlib.pyplot as plt





def estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation, activateDA):

    sourceReflectancesDA = image[np.nonzero(trainsegmentation)[0], :]
    # choose m reflectances for training DA
    m = trainingReflectances.shape[0]
    sourceReflectancesDA = np.matrix(random.sample(sourceReflectancesDA, m))

    #%% 2. determine domain adaptation weights

    trainingWeights = np.ones(trainingReflectances.shape[0])

    if (activateDA):
        trainingWeights = calculateWeights(trainingReflectances, sourceReflectancesDA)

    #%% 3. train forest

    rf = randomForest(trainingParameters, trainingReflectances, trainingWeights)

    #%% 4. estimate the parameters for the image

    print "starting to estimate the tissue parameters"
    start = time.time()

    estimatedParameters = rf.predict(image)

    # set to zero if not in segmentation mask
    #estimatedParameters[np.where(0 == testsegmentation), :] = 0

    end = time.time()
    print "time necessary to estimate parameters for image [s]: " + str((end - start))


    #%% save the parametric images TODO delete after everything works
#    import Image
#
#    for i in np.arange(0,estimatedParameters.shape[1]):
#        parameterImage_i = np.reshape(estimatedParameters[:,i], shape)
#        im = Image.fromarray(parameterImage_i)
#        im.save("data/output/" + "parameterImage_" + str(i) + ".tiff")


    #%% 6. evaluate data

    # for this, create monte carlo simulation for each
    # parameter estimate. The resulted reflectance estimate can then be compared to
    # the measured reflectance.

    from setup import systemPaths
    from setup import simulation

    import helper.monteCarloHelper as mch



    infileString, outfolderMC, outfolderRS, gpumcmlDirectory, gpumcmlExecutable = systemPaths.initPaths()
    infile = open(infileString)

    BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations = simulation.noisy()

    # the estimated parameters within the segmentation
    estimatedParametersOnlySegmented    = estimatedParameters[np.nonzero(testsegmentation)[0], :]
    # the image reflectances from which these parameters where estimated
    inputReflectancesOnlySegmented      = image[np.nonzero(testsegmentation)[0], :]

    # index vector for selecting n samples from this data
    indices = np.arange(0, estimatedParametersOnlySegmented.shape[0], 1)
    # choose n
    n = 20
    nSamples = random.sample(indices, n)
    estimatedParametersOnlySegmented = estimatedParametersOnlySegmented[nSamples]
    inputReflectancesOnlySegmented   = inputReflectancesOnlySegmented[nSamples]

    # placeholder for the reflectance computed from MC with the estimated parameters
    reflectancesFromEstimatedParameters = np.zeros((inputReflectancesOnlySegmented.shape[0], inputReflectancesOnlySegmented.shape[1]+1))

    #wavelengths = np.delete(wavelengths, [2, 7])

    for i, (BVF, Vs, d) in enumerate(estimatedParametersOnlySegmented):


        print('starting simulation ' + str(i) + ' of ' + str(estimatedParametersOnlySegmented.shape[0]))

        for j, wavelength in enumerate(wavelengths):

            reflectanceValue = mch.runOneSimulation(
                wavelength, eHbO2, eHb,
                infile, outfolderMC, gpumcmlDirectory, gpumcmlExecutable,
                BVF, Vs, d,
                # np.mean(rs), SaO2,
                # submucosa_BVF=sm_BVF, submucosa_Vs=sm_Vs, submucosa_SaO2=SaO2,
                Fwhm = FWHM, nrPhotons=photons)


            #print((BVF, Vs, d, wavelength))
            reflectancesFromEstimatedParameters[i, j] = reflectanceValue


    # correct these reflectances by image quotient
    reflectancesFromEstimatedParameters = mch.normalizeImageQuotient(reflectancesFromEstimatedParameters)

    wavelengths = mch.removeIqWavelength(wavelengths)
    #%% plot data for nicer inspection

    from sklearn.metrics      import r2_score

    r2Score = r2_score(reflectancesFromEstimatedParameters.T, inputReflectancesOnlySegmented.T)


    print("r2Score for random forest estimatation of:", str(r2Score))


    #%% sort by wavelength:

    for plot_i in range(n):

        sortedIndices = sorted(range(len(wavelengths)), key=lambda k: wavelengths[k])

        plt.figure()
        plt.plot(wavelengths[sortedIndices], reflectancesFromEstimatedParameters[plot_i,sortedIndices], 'g-o')
        plt.plot(wavelengths[sortedIndices], inputReflectancesOnlySegmented[plot_i,sortedIndices], 'b-o')
        print(str(r2_score(reflectancesFromEstimatedParameters[plot_i, :], inputReflectancesOnlySegmented[plot_i, :])))
        plt.legend(["estimated", "measurement"])
        plt.xlabel("wavelength [m]")
        plt.ylabel("normalized reflectance")
        plt.savefig("data/output/example_fit_" + str(plot_i) + '.png')

    return estimatedParameters, r2Score, reflectancesFromEstimatedParameters, inputReflectancesOnlySegmented