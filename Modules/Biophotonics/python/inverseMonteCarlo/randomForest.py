# -*- coding: utf-8 -*-
"""
Created on Fri Feb  6 10:49:45 2015

This is basically a wrapper for the python random forests.
It is necessary to do our preprocessing and KFold cross validation + grid search.

@author: wirkert
"""


import numpy as np
import time
from sklearn.ensemble         import RandomForestRegressor
from sklearn.grid_search      import GridSearchCV
from sklearn.cross_validation import KFold



# additional things that could be checked in this study:
# 1. band selection results
# 2. effect of normalizations
# 3. parameter study
# 4. optimal image quotient

def randomForest(trainingParameters, trainingReflectances, trainingWeights):
    #%% train forest

    start = time.time()

    # get best forest using k-fold cross validation and grid search
    # outcommented for now because it takes to long for quick tests

    print "starting forest training now."

#    kf = KFold(trainingReflectances.shape[0], 5, shuffle=True)
#    param_grid = [
#      {'max_depth': np.arange(2,40,1), 'max_features': np.arange(1,trainingReflectances.shape[1],1)}]
#
#    rf = GridSearchCV(RandomForestRegressor(500, max_depth=8), param_grid, cv=kf, n_jobs=11)
    #print("best random forest parameters: " + str(rf.best_estimator_))
    rf = RandomForestRegressor(500, max_depth=24)
    rf.fit(trainingReflectances, trainingParameters, trainingWeights)


    end = time.time()
    print "time necessary to train the forest [s]: " + str((end - start))


    return rf


