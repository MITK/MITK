'''
Created on Sep 8, 2015

@author: wirkert
'''

import math
import numpy as np

from scipy.interpolate import interp1d

import mc.bhmie_herbert_kaiser_july2012 as bhmie

def get_extinction_coefficients(reference_filename=None):
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


class Ua(object):

    def __init__(self):
        self.bvf = 0.02  # %
        self.cHb = 120.  # g*Hb/l
        self.saO2 = 0.  # %
        self.eHbO2, self.eHb = \
            get_extinction_coefficients()


    def __call__(self, wavelength):
        """ determine ua [1/m] as combination of
        Haemoglobin extinction coefficients.
        For more on this equation, please refer to
        http://omlc.org/spectra/hemoglobin/
        """
        return math.log(10) * self.cHb * \
            (self.saO2 * self.eHbO2(wavelength) +
             (1 - self.saO2) * self.eHb(wavelength)) \
            / 64500. * self.bvf


class UsG(object):

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
        sizeParameter = 2. * math.pi * self.r / wavelength
        nRelative = self.n_particle / self.n_medium
        # %% execute mie and create derived parameters
        s1, s2, qext, qsca, qback, gsca = bhmie.bhmie(sizeParameter,
                                                      nRelative, 900)
        vol_s = 4.*math.pi / 3.*(self.r ** 3)
        N_s = self.dsp / vol_s
        sigma_s = qsca * math.pi * (self.r ** 2)
        us = N_s * sigma_s
        g = gsca
#         A = math.pi * self.r ** 2  # geometrical cross sectional area
#         cs = qsca * A  # scattering cross section
#         # scattering coefficient [m-1]
#         us = self.dsp / (4. / 3. * self.r ** 3. * math.pi) * cs
#         g = gsca
        return us, g
