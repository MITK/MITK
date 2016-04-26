# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 13:42:00 2015

@author: wirkert
"""

import logging
import copy
import numpy as np

from scipy.interpolate import interp1d

from imgmani import collapse_image
import imgmani
from msi import Msi


'''
The msi manipulations module includes usefull convenience operations on msis.

E.g. calculate_mean_spectrum to calculate the average spectrum on all the image
data or interpolate_wavelengths to change to a different wavelenght set by
simple interpolation.

All methods take a msi and change it in place. They also return the same msi
object for convenience (can e.g. be used to chain msi manipulations).
'''


def apply_segmentation(msi, segmentation):
    """ applies a segmentation to an msi.

     If the msi has imaging data of n x m x nr_wavelengths the segmentation
     has to be a numpy array of n x m size. pixelvalues with values different
     than zero will be included in the segmentation.
     By applying the segmentation, not segmented elements will be np.ma.masked.

     Alternatively, one can input a msi with the mentioned n x m numpy array
     in from of a msi as segmentation (for convenience to be able to use the
     same reader for msis and segmentations)
     """
    if (isinstance(segmentation, Msi)):
        # expects just an image, but if a Msi is passed it's also ok
        segmentation = segmentation.get_image()
    segmentation = np.squeeze(segmentation)
    mask = (0 == segmentation)
    # mask needs to be expanded to cover all wavlengths
    wholeMask = np.zeros_like(msi.get_image(), dtype="bool")
    # doesn't seem elegant
    for i in range(wholeMask.shape[-1]):
        wholeMask[:, :, i] = mask

    msi.set_mask(wholeMask)
    return msi


def calculate_mean_spectrum(msi):
    """ reduce this image to only its mean spectrum.
    If the msi.get_image() is a masked array these values will be ignored when
    calculating the mean spectrum """
    # reshape to collapse all but last dimension (which contains reflectances)
    collapsedImage = collapse_image(msi.get_image())
    msi.set_image(np.mean(collapsedImage, axis=0))
    # returns the same msi.
    return msi


def interpolate_wavelengths(msi, newWavelengths):
    """ interpolate image data to fit newWavelengths. Current implementation
    performs simple linear interpolation. Neither existing nor new wavelengths
    need to be sorted. """
    interpolator = interp1d(msi.get_wavelengths(), msi.get_image(), assume_sorted=False)
    msi.set_image(interpolator(newWavelengths), wavelengths=newWavelengths)
    return msi


def normalize_integration_times(msi):
    """ divides by integration times """
    if ('integration times' not in msi.get_properties()):
        logging.warn("warning: trying to normalize integration times for "
            "image without the integration time property")
        return msi

    original_shape = msi.get_image().shape
    collapsed_image = collapse_image(msi.get_image())
    collapsed_image = collapsed_image / msi.get_properties()['integration times']
    msi.set_image(collapsed_image.reshape(original_shape))

    msi.add_property({'integration times':
        np.ones_like(msi.get_properties()['integration times'])})
    return msi


def dark_correction(msi, dark):
    """" subtract dark current from multi spectral image.

    The dark msi should either be of the same shape
    as msi or 1xnr_wavelengths (see tests)."""
    msi.set_image(msi.get_image() - dark.get_image())
    return msi


def flatfield_correction(msi, flatfield):
    """ divide by flatfield to remove dependencies on light source form and
    imaging system.

    The flatfield msi should either be of the same shape
    as msi or 1xnr_wavelengths (see tests)."""
    # we copy the flatfield to ensure it is unchanged by the normalization
    flatfield_copy = copy.copy(flatfield)
    normalize_integration_times(flatfield_copy)
    normalize_integration_times(msi)

    msi.set_image(msi.get_image() / flatfield_copy.get_image())
    return msi


def image_correction(msi, flatfield, dark):
    """ do the usual image correction:
    msi = ((msi - dark) / integration_time) / ((flatfield - dark) / integration_time)
    this function changes only the msi, flatfield and dark image
    are left unchanged.
    """
    # we need a copy of flatfield since otherwise the dark correction
    # changes the flatfield
    dark_correction(msi, dark)
    flatfield_copy = copy.copy(flatfield)
    dark_correction(flatfield_copy, dark)
    flatfield_correction(msi, flatfield_copy)
    return msi


def get_bands(msi, bands):
    """
    TODO SW: document and test
    """
    msi.set_image(imgmani.get_bands(msi.get_image(), bands))
    if msi.get_wavelengths() is not None:
        msi.set_wavelengths(msi.get_wavelengths()[bands])
    return msi

