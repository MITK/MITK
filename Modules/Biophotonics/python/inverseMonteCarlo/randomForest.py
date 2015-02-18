# -*- coding: utf-8 -*-
"""
Created on Fri Feb  6 10:49:45 2015

@author: wirkert
"""


import numpy as np
import time
from sklearn.ensemble         import RandomForestRegressor
from sklearn.grid_search      import GridSearchCV
from sklearn.cross_validation import KFold
from sklearn import decomposition

from setup import data




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
    data.perfect(dataFolder)

# transform data with pca (test)
#pca = decomposition.PCA(n_components=22)
#pca.fit(trainingReflectances)
#trainingReflectances = pca.transform(trainingReflectances)
#testReflectances     = pca.transform(testReflectances)

#%% train forest

start = time.time()

# get best forest using k-fold cross validation and grid search

#kf = KFold(trainingReflectances.shape[0], 5, shuffle=True)
#param_grid = [
#  {'max_depth': np.arange(2,20,1), 'n_estimators': np.arange(10,1000,10)}]

#best_rf = GridSearchCV(RandomForestRegressor(50, max_depth=8), param_grid, cv=kf, n_jobs=11)
#best_rf.fit(trainingReflectances, trainingParameters)


end = time.time()
print "time necessary to train the forest [s]: " + str((end - start))

# train a baseline forest

rf = RandomForestRegressor(50, max_depth=8, n_jobs=5)
rf.fit(trainingReflectances, trainingParameters)


#with open("iris.dot", 'w') as f:
#    f = tree.export_graphviz(rf, out_file=f)

#%% test


# predict test reflectances and get absolute errors.
absErrors = np.abs(rf.predict(testReflectances) - testParameters)
print("Baseline random forest with 50 trees and depth of 8")
print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(absErrors, axis=0)))
print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))


print("-------------------------------")

#print("result of random forest with parameters: " + str(best_rf.best_params_))
# predict test reflectances and get absolute errors.
#absErrors = np.abs(best_rf.predict(testReflectances) - testParameters)
#print("error distribution BVF, Volume fraction")
#print("median: " + str(np.median(absErrors, axis=0)))
#print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
#print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))


