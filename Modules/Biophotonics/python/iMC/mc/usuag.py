'''
Created on Sep 8, 2015

@author: wirkert
'''

import math
import os

import numpy as np
from scipy.interpolate import interp1d


this_dir, this_filename = os.path.split(__file__)
DATA_PATH = os.path.join(this_dir, "data")

def get_haemoglobin_extinction_coefficients(reference_filename=None):
    """
    helper method to get reference data for eHbO2 and eHb from Scott Prahls
    reference file:
    http://omlc.org/spectra/hemoglobin/summary.html
    """
    if reference_filename is None:
        reference_filename = os.path.join(DATA_PATH, "haemoglobin.txt")
    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt(reference_filename, skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:, 0] = haemoLUT[:, 0] * 10 ** -9
    haemoLUT[:, 1:] = haemoLUT[:, 1:] * 10 ** 2
    # get the data into an interpolation map for oxy and deoxy haemoglobin
    eHbO2 = interp1d(haemoLUT[:, 0], haemoLUT[:, 1])
    eHb = interp1d(haemoLUT[:, 0], haemoLUT[:, 2])
    return eHbO2, eHb


def get_beta_carotin_extinction_coefficients(reference_filename=None):
    """
    Reference data taken from
    http://omlc.org/spectra/PhotochemCAD/data/041-abs.txt
    """
    if reference_filename is None:
        reference_filename = os.path.join(DATA_PATH, "beta_carotin.txt")
    # table with wavelength at 1st row,
    # beta carotin molar extinction coefficient [cm**-1/(M)]
    betaLUT = np.loadtxt(reference_filename, skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    betaLUT[:, 0] = betaLUT[:, 0] * 10 ** -9
    betaLUT[:, 1:] = betaLUT[:, 1:] * 10 ** 2
    # get the data into an interpolation map
    eBc = interp1d(betaLUT[:, 0], betaLUT[:, 1], bounds_error=False,
                  fill_value=0.)
    return eBc


def get_bilirubin_extinction_coefficients(reference_filename=None):
    """
    Reference data taken from
    http://omlc.org/spectra/PhotochemCAD/data/041-abs.txt
    """
    if reference_filename is None:
        reference_filename = os.path.join(DATA_PATH, "bilirubin.txt")
    # table with wavelength at 1st row,
    # beta carotin molar extinction coefficient [cm**-1/(M)]
    biliLUT = np.loadtxt(reference_filename, skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    biliLUT[:, 0] = biliLUT[:, 0] * 10 ** -9
    biliLUT[:, 1:] = biliLUT[:, 1:] * 10 ** 2
    # get the data into an interpolation map
    eBili = interp1d(biliLUT[:, 0], biliLUT[:, 1], bounds_error=False,
                  fill_value=0.)
    return eBili


class Ua(object):

    def __init__(self):
        self.bvf = 0.02  # %
        self.cHb = 120.  # g*Hb/l
        self.saO2 = 0.  # %
        self.eHbO2, self.eHb = \
            get_haemoglobin_extinction_coefficients()

        self.cBetaCarotinUgProDl = 0.  # 2000.
        # g / l
        self.cBili = 0.  # 1.23 * 10 ** -2
        self.eBc = get_beta_carotin_extinction_coefficients()
        self.eBili = get_bilirubin_extinction_coefficients()


    def __call__(self, wavelength):
        """ determine ua [1/m] as combination of
        Haemoglobin extinction coefficients.
        For more on this equation, please refer to
        http://omlc.org/spectra/hemoglobin/
        """
        ua_haemoglobin = math.log(10) * self.cHb * \
            (self.saO2 * self.eHbO2(wavelength) +
            (1 - self.saO2) * self.eHb(wavelength)) \
            / 64500. * self.bvf
        ua_bilirubin = math.log(10) * self.cBili / 574.65 * \
            self.eBili(wavelength)
        # second line is to convert from ug/dl to g/ mole
        ua_beta_carotin = math.log(10) * self.cBetaCarotinUgProDl / \
            536.8726 * 10 ** -5 * \
            self.eBc(wavelength)

        return ua_haemoglobin + ua_bilirubin + ua_beta_carotin


class UaMuscle():
    """helper class for setting ua in muscle layer.
    for ua_sm in the muscle layer we don't use mie theory but the
    approximation presented in
    Rowe et al.
    "Modelling and validation of spectral reflectance for the colon"
    calculated to retrieve an absorption of 11.2 cm-1 at 515nm
    """
    def __init__(self):
        self.ua = Ua()

    def __call__(self, wavelength):
        A = 1.7923385088285804
        self.ua.bvf = 0.1 * A
        self.ua.saO2 = 0.7
        self.ua.cHb = 120.
        return self.ua(wavelength)


class UsgJacques(object):

    def __init__(self):
        """
        To be set externally:

        a':
        """
        self.a_ray = 0. * 100.
        self.a_mie = 20. * 100.
        self.b_mie = 1.286
        self.g = 0.

    def __call__(self, wavelength):
        """
        Calculate the scattering parameters relevant for monte carlo simulation.

        Uses equation (2) from: Optical properties of biological tissues:
        a Review

        Args
        ____
        wavelength:
            wavelength of the incident light [m]

        Returns:
        ____
        (us, g)
            scattering coefficient us [1/m] and anisotropy factor g
        """
        norm_wavelength = (wavelength / (500 * 10 ** -9))

        us_ray = self.a_ray * norm_wavelength ** (-4)
        us_mie = self.a_mie * norm_wavelength ** (-self.b_mie)

        us_prime = (us_ray + us_mie)  # * 100. to convert to m^-1
        # actually we calculated the reduced scattering coefficent, so
        # assume g is 0
        us = us_prime / (1 - self.g)

        return us, self.g


class UsGMuscle(object):
    """helper object for setting us in muscle layer.
    for us in the muscle layer we don't use mie theory but the
    approximation presented in
    Rowe et al.
    "Modelling and validation of spectral reflectance for the colon"
    """

    def __init__(self):
        pass


    def __call__(self, wavelength):
        us = 168.52 * (wavelength * 10 ** 9) ** -0.332 / (1. - 0.96) * 100.
        g = 0.96
        return us, g


class UsgIntralipid(object):
    """helper object for setting us and g in intralipid
    We use the formulas from
    http://omlc.org/spectra/intralipid/ to calculate
    """

    def __init__(self):
        self.a_ray = 0. * 100.
        self.a_mie = 20. * 100.
        self.b_mie = 2.33
        self.g = 0.85

    def __call__(self, wavelength):

        norm_wavelength = (wavelength / (500 * 10 ** -9))

        us_ray = self.a_ray * norm_wavelength ** (-4)
        us_mie = self.a_mie * norm_wavelength ** (-self.b_mie)

        us_prime = (us_ray + us_mie)  # * 100. to convert to m^-1

        g = 2.25 * (wavelength * 10**9)**-0.155

        us = us_prime / (1 - g)

        return us, g
