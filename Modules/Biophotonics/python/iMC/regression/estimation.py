'''
Created on Oct 21, 2015

@author: wirkert
'''

import math
import logging
import time

import numpy as np
import SimpleITK as sitk


import msi.imgmani as imgmani


def SAMDistance(x, y):
    return math.acos(np.dot(x, y) / (np.linalg.norm(x) * np.linalg.norm(y)))


def estimate_image(msi, regressor):
    """given an Msi and an regressor estimate the parmaeters for this image

    Paramters:
        msi: multi spectral image
        regressor: regressor, must implement the predict method"""

    # estimate parameters
    collapsed_msi = imgmani.collapse_image(msi.get_image())
    # in case of nan values: set to 0
    collapsed_msi[np.isnan(collapsed_msi)] = 0.
    collapsed_msi[np.isinf(collapsed_msi)] = 0.

    start = time.time()
    estimated_parameters = regressor.predict(collapsed_msi)
    end = time.time()
    estimation_time = end - start
    logging.info("time necessary for estimating image parameters: " +
        str(estimation_time) + "s")
    # restore shape
    feature_dimension = 1
    if len(estimated_parameters.shape) > 1:
        feature_dimension = estimated_parameters.shape[-1]

    estimated_paramters_as_image = np.reshape(
            estimated_parameters, (msi.get_image().shape[0],
                                   msi.get_image().shape[1],
                                   feature_dimension))
    # save as sitk nrrd.
    sitk_img = sitk.GetImageFromArray(estimated_paramters_as_image,
                                 isVector=True)
    return sitk_img, estimation_time

def standard_score(estimator, X, y):
    """our standard scoring method is the median absolute error"""
    return np.median(np.abs(estimator.predict(X) - y))
