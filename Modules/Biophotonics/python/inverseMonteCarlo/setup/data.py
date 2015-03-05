# -*- coding: utf-8 -*-
"""
Created on Fri Feb 13 16:13:11 2015

functions storing training / testdata combinations to ensure consistent usage
of datasets (same combinations / normalizations / ...)

@author: wirkert
"""

import numpy as np
import helper.monteCarloHelper as mch



def perfect(dataFolder):
    trainingParameters   = np.load(dataFolder + "2015February1612:04AMparamtersPerfectArtificial.npy")
    trainingParameters   = trainingParameters[:,0:2] # only BVF and Vs for perfect data simulations
    trainingReflectances = np.load(dataFolder + "2015February1612:04AMreflectancesPerfectArtificial1000000photons.npy")
    trainingReflectances = mch.normalizeImageQuotient(trainingReflectances, iqBand=1)

    testParameters   = np.load(dataFolder + "2015February1702:02AMparamtersRandom.npy")
    testParameters   = testParameters[:,0:2] # only BVF and Vs for perfect data simulations
    testReflectances = np.load(dataFolder + "2015February1702:02AMreflectancesRandom.npy")
    testReflectances = mch.normalizeImageQuotient(testReflectances, iqBand=1)

    return trainingParameters, trainingReflectances, testParameters, testReflectances



def logisticRegressionArtificialData(dataFolder):
    uniformReflectances = np.load(dataFolder + "2015February1704:00PMreflectancesRandom1000000photons.npy")
    uniformReflectances = mch.normalizeImageQuotient(uniformReflectances, iqBand=1)

    gaussReflectances = np.load(dataFolder + "2015February1703:56PMreflectancesRandomNonUniform1000000photons.npy")
    gaussReflectances = mch.normalizeImageQuotient(gaussReflectances, iqBand=1)

    return uniformReflectances, gaussReflectances

