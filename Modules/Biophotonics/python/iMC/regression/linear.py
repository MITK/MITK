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

    def __init__(self):
        # oxygenated haemoglobin extinction coefficients
        eHb02 = 0
        eHb = 0

        # oxygenated haemoglobin extinction coefficients
        eHbO2 = np.array([34772.8,
                          27840.93333,
                          23748.8    ,
                          21550.8    ,
                          21723.46667,
                          28064.8    ,
                          39131.73333,
                          45402.93333,
                          42955.06667,
                          40041.73333,
                          42404.4    ,
                          36333.6    ,
                          22568.26667,
                          6368.933333,
                          1882.666667,
                          1019.333333,
                          664.6666667,
                          473.3333333,
                          376.5333333,
                          327.2      ,
                          297.0666667],)
        # deoxygenated haemoglobin extinction coefficients
        eHb = [18031.73333 ,
               15796.8     ,
               17365.33333 ,
               21106.53333 ,
               26075.06667 ,
               32133.2     ,
               39072.66667 ,
               46346.8     ,
               51264       ,
               50757.33333 ,
               45293.33333 ,
               36805.46667 ,
               26673.86667 ,
               17481.73333 ,
               10210.13333 ,
               7034        ,
               5334.533333 ,
               4414.706667 ,
               3773.96     ,
               3257.266667 ,
               2809.866667]
        nr_total_wavelengths = len(eHb)
        # to account for scattering losses we allow a constant offset
        scattering = np.ones(nr_total_wavelengths)
        # put eHbO2, eHb and scattering term in one measurement matrix
        self.H = np.vstack((eHbO2, eHb, scattering)).T
        self.lsq_solution_matrix = np.dot(np.linalg.inv(np.dot(self.H.T,
                                                               self.H)),
                                          self.H.T)


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
