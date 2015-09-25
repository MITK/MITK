'''
Created on Sep 8, 2015

@author: wirkert
'''

import math
import numpy as np

from scipy.interpolate import interp1d

import mc.bhmie_herbert_kaiser_july2012 as bhmie

def get_haemoglobin_extinction_coefficients(reference_filename=None):
    """
    helper method to get reference data for eHbO2 and eHb from Scott Prahls
    reference file:
    http://omlc.org/spectra/hemoglobin/summary.html
    """
    if reference_filename is None:
        reference_filename = "./mc/data/haemoglobin.txt"
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
        reference_filename = "./mc/data/beta_carotin.txt"
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
        reference_filename = "./mc/data/bilirubin.txt"
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

        self.cBetaCarotinUgProDl = 20.8
        # g / l
        self.cBili = 0.0123
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
        ua_beta_carotin = math.log(10) * self.cBetaCarotinUgProDl * \
            0.01 / 536.8726 * 10 ** -3 * \
            self.eBc(wavelength) * self.bvf * 0.55  # 55 % of blood is plasma

        return  ua_haemoglobin + ua_bilirubin + ua_beta_carotin




class UsgJacques(object):

    def __init__(self):
        """
        To be set externally:

        a':
        """
        self.a_ray = 0. / 100.
        self.a_mie = 20. / 100.
        self.b_mie = 1.091

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

        us = (us_ray + us_mie)  # * 100. to convert to m^-1
        # actually we calculated the reduced scattering coefficent, so
        # assume g is 0
        g = 0

        return us, g




class UsgMie(object):

    def __init__(self):
        """
        To be set externally:

        r:
            radius of the particle [m]
        dsp:
            volume fraction of scattering particles
        n_particle:
            refractive index of the particle that the light wave is scattered on
            (default value is the refractive index of collagen)
        n_medium:
            refractive index of the surronding medium
            (default is that of colonic mucosal tissue)
        """
        self.r = 0.4 * 10 ** -6
        self.dsp = 0.1
        self.n_particle = 1.46
        self.n_medium = 1.36

    def __call__(self, wavelength):
        """
        Calculate the scattering parameters relevant for monte carlo simulation.

        Needs pymiecoated: https://code.google.com/p/pymiecoated/
        Also see http://omlc.org/education/ece532/class3/musdefinition.html
        for an explanation on how the scattering coefficient is derived

        Args
        ____
        wavelength:
            wavelength of the incident light [m]

        Returns:
        ____
        (us, g)
            scattering coefficient us [1/m] and anisotropy factor g
        """
        # create derived parameters
#         sizeParameter = 2. * math.pi * self.r / wavelength
#         nRelative = self.n_particle / self.n_medium
        # %% execute mie and create derived parameters
        s1, s2, qext, qsca, qback, gsca = bhmie.bhmie(self.r, wavelength,
                                                      self.n_particle,
                                                      self.n_medium, 900)
        vol_s = 4.*math.pi / 3.*(self.r ** 3)
        N_s = self.dsp * 1.0 / (vol_s)
        sigma_s = qsca * math.pi * (self.r ** 2)
        us = N_s * sigma_s
        g = gsca
#         A = math.pi * self.r ** 2  # geometrical cross sectional area
#         cs = qsca * A  # scattering cross section
#         # scattering coefficient [m-1]
#         us = self.dsp / (4. / 3. * self.r ** 3. * math.pi) * cs
#         g = gsca
        return us, g

