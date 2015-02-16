# -*- coding: utf-8 -*-
"""
Created on Fri Feb 13 16:13:11 2015

functions storing training / testdata combinations to ensure consistent usage
of datasets (same combinations / normalizations / ...)

@author: wirkert
"""

import numpy as np
import monteCarloHelper as mch

def setupTwoDimensionalData(dataFolder):
    trainingParameters   = np.load(dataFolder + "2015February0511:02PMparamters2D.npy")
    trainingReflectances = np.load(dataFolder + "2015February0511:02PMreflectances2D.npy")
    # trainingReflectances = mch.normalizeImageQuotient(trainingReflectances, iqBand=4)

    testParameters   = np.load(dataFolder + "2015February1107:43PMparamtersRandom2D.npy")
    testReflectances = np.load(dataFolder + "2015February1107:43PMreflectancesRandom2D.npy")
    # testReflectances = mch.normalizeImageQuotient(testReflectances, iqBand=4)

    return trainingParameters, trainingReflectances, testParameters, testReflectances




