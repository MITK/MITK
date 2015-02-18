# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 16:14:48 2015

estimate the weighting function for performing the domain adaption.

@author: wirkert
"""


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
uniformReflectances, labelsUniform, gaussReflectances, labelsGauss = \
    data.logisticRegressionArtificialData(dataFolder)


allReflectances = np.concatenate((uniformReflectances, gaussReflectances))
allLabels       = np.concatenate((labelsUniform, labelsGauss))


# train logistic regression

start = time.time()

#lr = LogisticRegression()


kf = KFold(allReflectances.shape[0], 2, shuffle=True)
# todo include intercept scaling paramter
param_grid = [
  {'C': np.logspace(-3,6,1000), 'fit_intercept':['True', 'False']} ]

best_lr = GridSearchCV(LogisticRegression(), param_grid, cv=kf, n_jobs=11)

best_lr.fit(allReflectances, allLabels)

end = time.time()
print "time necessary to train the logistic regression [s]: " + str((end - start))


#%% test


