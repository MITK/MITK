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



# additional things that could be checked in this study:
# 1. band selection results
# 2. effect of normalizations
# 3. parameter study
# 4. optimal image quotient

def randomForest(trainingParameters, trainingReflectances, trainingWeights, testParameters, testReflectances):
    #%% train forest

    start = time.time()

    # transform data with pca (test)
    pca = decomposition.PCA(n_components=trainingReflectances.shape[1])
    pca.fit(trainingReflectances)
    trainingReflectances = pca.transform(trainingReflectances)
    testReflectances     = pca.transform(testReflectances)

    # get best forest using k-fold cross validation and grid search
    # outcommented for now because it takes to long for quick tests

    kf = KFold(trainingReflectances.shape[0], 5, shuffle=True)
    param_grid = [
      {'max_depth': np.arange(2,40,1), 'n_estimators': np.logspace(1,11,11,base=2).astype(int)}]

    rf = GridSearchCV(RandomForestRegressor(50, max_depth=8), param_grid, cv=kf, n_jobs=11)
    rf.fit(trainingReflectances, trainingParameters)


    end = time.time()
    print "time necessary to train the forest [s]: " + str((end - start))

    # train a baseline forest

    #rf = RandomForestRegressor(100, max_depth=10, n_jobs=5)
    #rf.fit(testReflectances, testParameters)

    #with open("iris.dot", 'w') as f:
    #    f = tree.export_graphviz(rf, out_file=f)

    # predict test reflectances and get absolute errors.
    absErrors = np.abs(rf.predict(testReflectances) - testParameters)

    return (rf, absErrors, rf.score(testReflectances, testParameters))



