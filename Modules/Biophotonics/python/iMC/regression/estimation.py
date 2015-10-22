'''
Created on Oct 21, 2015

@author: wirkert
'''

import numpy as np
from sklearn.preprocessing import Imputer
import SimpleITK as sitk

import msi.imgmani as imgmani


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

    estimated_parameters = regressor.predict(collapsed_msi)
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
    return sitk_img
