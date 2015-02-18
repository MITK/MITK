# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 16:14:48 2015

estimate the weighting function for performing the domain adaption.

@author: wirkert
"""


import numpy as np
import time

import numpy as np

from sklearn.grid_search      import GridSearchCV
from sklearn.cross_validation import KFold
from sklearn.linear_model     import LogisticRegression

from setup import data


#%% initialize


# the folder with the reflectance spectra
dataFolder = 'data/output/'

# load data
uniformReflectances, gaussReflectances = \
    data.logisticRegressionArtificialData(dataFolder)

labelsUniform = np.zeros(uniformReflectances.shape[0])
labelsGauss   = np.ones(gaussReflectances.shape[0])

allReflectances = np.concatenate((uniformReflectances, gaussReflectances))
allLabels       = np.concatenate((labelsUniform, labelsGauss))

#%% train forest


# get best forest using k-fold cross validation and grid search

#kf = KFold(trainingReflectances.shape[0], 5, shuffle=True)
#param_grid = [
#  {'max_depth': np.arange(2,20,1), 'n_estimators': np.arange(10,1000,10)}]

#best_rf = GridSearchCV(RandomForestRegressor(50, max_depth=8), param_grid, cv=kf, n_jobs=11)
#best_rf.fit(trainingReflectances, trainingParameters)



# train logistic regression

start = time.time()

lr = LogisticRegression()
lr.fit(allReflectances, allLabels)

end = time.time()
print "time necessary to train the logistic regression [s]: " + str((end - start))


#with open("iris.dot", 'w') as f:
#    f = tree.export_graphviz(rf, out_file=f)

#%% test


