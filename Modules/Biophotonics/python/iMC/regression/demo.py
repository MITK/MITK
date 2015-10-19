'''
Created on Oct 19, 2015

@author: wirkert
'''

import pickle

import numpy as np

from linear import LinearSaO2Regression

if __name__ == '__main__':
    f = open("/media/wirkert/data/Data/" +
        "2015_06_01_Filtertransmittance_Spectrometer/processed/" +
        "visualization_saO2_all_camera.imc")
    data = pickle.load(f)

    regressor = LinearSaO2Regression()
    saO2s = regressor.predict(-np.log(data.reflectances))

    print "estimated saO2", saO2s
    print "real_saO2", data.layers[0][:, 1]


