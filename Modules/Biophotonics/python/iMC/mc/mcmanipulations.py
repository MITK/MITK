'''
Created on Oct 19, 2015

@author: wirkert
'''

import numpy as np
from scipy.interpolate import interp1d


def fold_by_sliding_average(batch, window_size):
    """take a batch and apply a sliding average with given window size to
    the reflectances.
    window_size is elements to the left and to the right.
    There will be some boundary effect on the edges."""
    sliding_average = lambda x: np.convolve(x,
                                            np.ones(window_size, dtype=float) /
                                            float(window_size), mode="same")
    batch.reflectances = np.apply_along_axis(sliding_average,
                                             axis=1,
                                             arr=batch.reflectances)


def interpolate_wavelengths(batch, new_wavelengths):
    """ interpolate image data to fit new_wavelengths. Current implementation
    performs simple linear interpolation. Neither existing nor new wavelengths
    need to be sorted. """
    interpolator = interp1d(batch.wavelengths,
                            batch.reflectances, assume_sorted=False)
    batch.reflectances = interpolator(new_wavelengths)
    batch.wavelengths = new_wavelengths
