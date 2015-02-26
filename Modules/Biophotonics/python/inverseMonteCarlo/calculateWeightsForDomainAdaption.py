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


def calculateWeights(sourceReflectances, targetReflectances):

    labelsSource = np.zeros(sourceReflectances.shape[0])
    labelsTarget = np.ones(targetReflectances.shape[0])

    allReflectances = np.concatenate((sourceReflectances, targetReflectances))
    allLabels       = np.concatenate((labelsSource, labelsTarget))

    # train logistic regression

    start = time.time()

    print "now starting domain adaptation"

    kf = KFold(allReflectances.shape[0], 5, shuffle=True)
    # todo include intercept scaling paramter
    param_grid = [
      {'C': np.logspace(-3,6,1000), 'fit_intercept':['True', 'False']} ]

    best_lr = GridSearchCV(LogisticRegression(), param_grid, cv=kf, n_jobs=11)

    best_lr.fit(allReflectances, allLabels)

    end = time.time()
    print "time necessary to train the logistic regression [s]: " + str((end - start))

    sourceProbabilities = best_lr.predict_proba(sourceReflectances)

    return sourceProbabilities[:,1] / sourceProbabilities[:,0]

    #%% test


    #print "score for uniform test:", best_lr.score(testReflectancesUniform, labelsTestUniform)
    #print "score for gauss test  :", best_lr.score(testReflectancesGauss, labelsTestGauss)

