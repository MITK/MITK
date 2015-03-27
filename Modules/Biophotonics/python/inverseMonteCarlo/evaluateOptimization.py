# -*- coding: utf-8 -*-
"""
Created on Tue Feb 24 09:38:09 2015

@author: wirkert
"""

import numpy as np
import matplotlib.pyplot as plt
from setup import data
from optimization import optimization

#%% load data

dataFolder = "data/output/"

trainingParameters, trainingReflectances, testParameters, testReflectances = \
    data.perfect(dataFolder)

dummy1, dummy2, testParameters, testReflectances = \
    data.noisy(dataFolder)

testingErrors, r2Score = optimization(trainingParameters, trainingReflectances, testParameters, testReflectances)

#%% test

print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(testingErrors, axis=0)))
print("lower quartile: " + str(np.percentile(testingErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(testingErrors, 75, axis=0)))
print("r2Score", str(r2Score))