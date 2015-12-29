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
    performs simple linear interpolation. Neither existing nor new _wavelengths
    need to be sorted. """
    interpolator = interp1d(batch._wavelengths,
                            batch.reflectances, assume_sorted=False,
                            bounds_error=False)
    batch.reflectances = interpolator(new_wavelengths)
    batch._wavelengths = new_wavelengths


def sortout_bands(batch, bands_to_sortout=None):
    """ TODO: Documentation and test """
    if bands_to_sortout is not None:
        # get rid of sorted out bands
        batch.reflectances = np.delete(batch.reflectances,
                                              bands_to_sortout, axis=1)
        batch._wavelengths = np.delete(batch._wavelengths,
                                              bands_to_sortout)


def select_n(batch, n):
    """ randomly select n elements from batch. TODO: Test """
    perms = np.random.permutation(batch.reflectances.shape[0])
    first_n_perms = perms[0:n]
    batch.reflectances = batch.reflectances[first_n_perms, :]
    for i, l in enumerate(batch.layers):
        batch.layers[i] = np.atleast_2d(l)[first_n_perms, :]
    return batch
