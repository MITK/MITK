# -*- coding: utf-8 -*-
"""
Created on Sat Feb 07 23:42:41 2015

@author: Seb
"""

import numpy as np
from scipy.optimize       import minimize
from reflectanceError     import ReflectanceError
from sklearn.metrics      import r2_score




def optimization(trainingParameters, trainingReflectances, testParameters, testReflectances):

    BVFs = np.unique(trainingParameters[:,0])
    Vss  = np.unique(trainingParameters[:,1])


    #%% build optimization function from rectangular reflectance grid

    reflectanceGrid3D  = np.reshape(trainingReflectances, (len(BVFs), len(Vss), trainingReflectances.shape[1]))
    functionToMinimize = ReflectanceError(BVFs, Vss, reflectanceGrid3D)


    #%% do optimization

    estimatedParameters = np.zeros_like(testParameters)
    absErrors           = np.zeros_like(testParameters)


    for idx, (testParameter, testReflectance) in enumerate(zip(testParameters, testReflectances)):
        functionToMinimize.setReflectanceToMatch(testReflectance)
        minimization = minimize(functionToMinimize.f, [np.median(BVFs), np.median(Vss)], method="Nelder-Mead")
        # interpolation extrapolates with constant values. We just crop it to the bounds
        clippedX= np.clip(minimization.x, [min(BVFs), min(Vss)], [max(BVFs), max(Vss)])

        estimatedParameters[idx, :] = clippedX
        absErrors[idx,:]            = np.abs(clippedX - testParameter)

    r2Score = r2_score(testParameters.T, estimatedParameters.T)

    return absErrors, r2Score

