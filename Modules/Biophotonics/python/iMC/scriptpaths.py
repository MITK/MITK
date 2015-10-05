'''
Created on Aug 27, 2015

@author: wirkert
'''

import numpy as np

ROOT_FOLDER = "/media/wirkert/data/Data/2015_08_18_FAP/organized_recordings"
FLAT_FOLDER = "flatfields"
DARK_FOLDER = "dark"
DATA_FOLDER = "colon_images"
SPECTROMETER_FOLDER = "spectrometer_measurements"
RESULTS_FOLDER = "processed"
FINALS_FOLDER = "finals"
MC_DATA_FOLDER = "mc_data"

RECORDED_WAVELENGTHS = \
        np.array([580, 470, 660, 560, 480, 511, 600, 700]) * 10 ** -9

# some hacky functions to adapt to certain measurement scenarios:

def resort_wavelengths(msi):
    """ as a standard, no resorting takes place. rebind this method
    if wavelengths need sorting """
    return msi

bands_to_sortout = []