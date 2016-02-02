'''
Created on Oct 19, 2015

@author: wirkert
'''

import numpy as np
from scipy.interpolate import interp1d

from mc.usuag import get_haemoglobin_extinction_coefficients

class LinearSaO2Unmixing(object):
    '''
    classdocs
    '''

    def __init__(self, use_LCTF=False):
        # oxygenated haemoglobin extinction coefficients
        eHbO2 = np.array([ \
            50104  ,
            33209.2,
            319.6  ,
            32613.2,
            26629.2,
            20035.2,
            3200
            , 290
        ])
        # deoxygenated haemoglobin extinction coefficients
        eHb = np.array([
            37020   ,
            16156.4 ,
            3226.56 ,
            53788   ,
            14550   ,
            25773.6 ,
            14677.2
            , 1794.28
        ])
        nr_total_wavelengths = len(eHb)
        # to account for scattering losses we allow a constant offset
        scattering = np.ones(nr_total_wavelengths)
        # put eHbO2, eHb and scattering term in one measurement matrix
        self.H = np.vstack((eHbO2, eHb, scattering)).T
        self.lsq_solution_matrix = np.dot(np.linalg.inv(np.dot(self.H.T,
                                                               self.H)),
                                          self.H.T)

        self.selected_features = np.arange(0, nr_total_wavelengths, 1)

    def set_selected_features(self, selected_features):
        """
        Parameters:
            selected_features: index array with indices
            of selected features. Example np.array([0, 3, 19]) would select
            the 1st, 4th, and 19th feature.
            Will be initialized by default to all features.
        """
        self.selected_features = selected_features

    def fit(self, X, y, weights=None):
        """only implemented to fit to the standard sklearn framework."""
        pass

    def predict(self, X):
        """predict like in sklearn:

        Parameters:
            X: nrsamples x nr_features matrix of samples to predict for
            regression

        Returns:
            y: array of shape [nr_samples] with values for predicted
            oxygenation """
        # do least squares estimation
        oxy_test, deoxy, s = np.dot(self.lsq_solution_matrix, X.T)
        # calculate oxygenation = oxygenated blood / total blood
        saO2 = oxy_test / (oxy_test + deoxy)

        return np.clip(saO2, 0., 1.)
