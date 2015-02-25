# -*- coding: utf-8 -*-
"""
Created on Fri Feb 13 16:13:11 2015

functions storing training / testdata combinations to ensure consistent usage
of datasets (same combinations / normalizations / ...)

@author: wirkert
"""

import numpy as np
import helper.monteCarloHelper as mch

import csvImageReader



def perfect(dataFolder):
    trainingParameters   = np.load(dataFolder + "2015February2401:43AMPerfectGridTraining10000parameters.npy")
    trainingParameters   = trainingParameters[:,0:2] # only BVF and Vs for perfect data simulations
    trainingReflectances = np.load(dataFolder + "2015February2401:43AMPerfectGridTrainingreflectances1000000photons.npy")
    trainingReflectances = mch.normalizeImageQuotient(trainingReflectances)

    testParameters   = np.load(dataFolder + "2015February2306:52PMUniformRandom10000parameters.npy")
    testParameters   = testParameters[:,0:2] # only BVF and Vs for perfect data simulations
    testReflectances = np.load(dataFolder + "2015February2306:52PMUniformRandomreflectances1000000photons.npy")
    testReflectances = mch.normalizeImageQuotient(testReflectances)

    return trainingParameters, trainingReflectances, testParameters, testReflectances


def noisy(dataFolder):
    trainingParameters   = np.load(dataFolder + "2015February2208:16PMNoisyRandomTraining10000parameters.npy")
    trainingParameters   = trainingParameters[:,0:4]
    trainingReflectances = np.load(dataFolder + "2015February2208:16PMNoisyRandomTrainingreflectances1000000photons.npy")
    trainingReflectances = mch.normalizeImageQuotient(trainingReflectances)

    testParameters   = np.load(dataFolder + "2015February2301:23AMNoisyRandomTesting10000parameters.npy")
    testParameters   = testParameters[:,0:4]
    testReflectances = np.load(dataFolder + "2015February2301:23AMNoisyRandomTestingreflectances1000000photons.npy")
    testReflectances = mch.normalizeImageQuotient(testReflectances)

    return trainingParameters, trainingReflectances, testParameters, testReflectances


def realImage(dataFolder, imageToLoad):
    trainingParameters   = np.load(dataFolder + "2015February2208:16PMNoisyRandomTraining10000parameters.npy")
    # estimate: BVF, Vs, d, SaO2:
    trainingParameters   = trainingParameters[:, 0:4]
    trainingReflectances = np.load(dataFolder + "2015February2208:16PMNoisyRandomTrainingreflectances1000000photons.npy")
    trainingReflectances = mch.normalizeImageQuotient(trainingReflectances)

    shape, image, trainsegmentation, testsegmentation = csvImageReader.csvMultiSpectralImageReader(dataFolder + imageToLoad)
    image        = mch.normalizeImageQuotient(image, 1)

    return trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation


def logisticRegressionArtificialData(dataFolder):
    uniformReflectances = np.load(dataFolder + "2015February1704:00PMreflectancesRandom1000000photons.npy")
    uniformReflectances = mch.normalizeImageQuotient(uniformReflectances)

    gaussReflectances = np.load(dataFolder + "2015February1908:54AMnonUniformRandomTrainingDatareflectances1000000photons.npy")
    gaussReflectances = mch.normalizeImageQuotient(gaussReflectances)

    testReflectancesGauss = np.load(dataFolder + "2015February1908:57AMnonUniformRandomTestingDatareflectances1000000photons.npy")
    testReflectancesGauss = mch.normalizeImageQuotient(testReflectancesGauss)


    return uniformReflectances, gaussReflectances, testReflectancesGauss

