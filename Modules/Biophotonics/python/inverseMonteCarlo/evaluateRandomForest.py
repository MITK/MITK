# -*- coding: utf-8 -*-
"""
Created on Tue Feb 24 09:45:05 2015

@author: wirkert
"""

import numpy as np

from setup import data
from randomForest import randomForest


# the folder with the reflectance spectra
dataFolder = 'data/output/'

# load data
trainingParameters, trainingReflectances, testParameters, testReflectances = \
    data.perfect(dataFolder)

trainingWeights = np.ones((trainingParameters.shape[0], 1))

rf, testingErrors, r2Score = randomForest(trainingParameters, trainingReflectances, trainingWeights, testParameters, testReflectances)

#%% test


#import matplotlib.pyplot as plt

print("Baseline random forest with 100 trees and depth of 8")
print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(testingErrors, axis=0)))
print("lower quartile: " + str(np.percentile(testingErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(testingErrors, 75, axis=0)))


#print("result of random forest with parameters: " + str(best_rf.best_params_))
# predict test reflectances and get absolute errors.
#absErrors = np.abs(best_rf.predict(testReflectances) - testParameters)
#print("error distribution BVF, Volume fraction")
#print("median: " + str(np.median(absErrors, axis=0)))
#print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
#print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))
