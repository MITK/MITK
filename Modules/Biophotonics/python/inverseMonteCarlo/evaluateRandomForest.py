# -*- coding: utf-8 -*-
"""
Created on Tue Feb 24 09:45:05 2015

@author: wirkert
"""

import numpy as np

from setup import data
from randomForest import randomForest


#%% load data
# the folder with the reflectance spectra
dataFolder = 'data/output/'

trainingParameters, trainingReflectances, testParameters, testReflectances = \
    data.noisy(dataFolder)


trainingWeights = np.ones((trainingParameters.shape[0],))

#%% train forest

rf = randomForest(trainingParameters, trainingReflectances, trainingWeights)

#%% test

#with open("iris.dot", 'w') as f:
#    f = tree.export_graphviz(rf, out_file=f)

# predict test reflectances and get absolute errors.

absErrors = np.abs(rf.predict(testReflectances) - testParameters)

r2Score = rf.score(testReflectances, testParameters)

#import matplotlib.pyplot as plt

print("absolute error distribution BVF, Volume fraction")
print("median: " + str(np.median(absErrors, axis=0)))
print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))
print("r2Score", str(r2Score))
