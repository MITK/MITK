# -*- coding: utf-8 -*-
"""
Created on Fri Feb  6 10:49:45 2015

@author: wirkert
"""


import numpy as np
import matplotlib.pyplot as plt
from sklearn          import tree
from sklearn.ensemble import RandomForestRegressor

import setupData



# additional things that could be checked in this study:
# 1. band selection results
# 2. effect of normalizations
# 3. parameter study
# 4. optimal image quotient

#%% initialize


# the folder with the reflectance spectra
dataFolder = 'data/output/'

# load data
trainingParameters, trainingReflectances, testParameters, testReflectances = \
    setupData.setupTwoDimensionalData(dataFolder)


#%% train forest

rf = RandomForestRegressor(50, max_depth=8, max_features="log2", n_jobs=5)
rf.fit(trainingReflectances, trainingParameters)

#with open("iris.dot", 'w') as f:
#    f = tree.export_graphviz(rf, out_file=f)

#%% test


# predict test reflectances and get absolute errors.
absErrors = np.abs(rf.predict(testReflectances) - testParameters)

print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(absErrors, axis=0)))
print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))
