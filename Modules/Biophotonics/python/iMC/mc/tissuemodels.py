'''
Created on Sep 9, 2015

@author: wirkert
'''

from mc.sim import MciWrapper
from mc.usuag import Ua

class Colon(object):
    '''
    Initializes a three layer colon tissue model as e.g. used by
    Rowe et al. "Modelling and validation of spectral reflectance for the colon"
    '''

    def set_mci_filename(self, mci_filename):
        self._mci_wrapper.set_mci_filename(mci_filename)

    def set_mucosa(self, ua, us, g, d):
        self._mci_wrapper.set_layer(1, 1.38, ua, us, g, d)

    def set_submucosa(self, ua, us, g, d=None):
        if d is None:
            d = 0.05
        self._mci_wrapper.set_layer(2, 1.36, ua, us, g, d)

    def set_muscle(self, ua, us, g=None, d=None):
        if d is None:
            d = 0.09
        if g is None:
            g = 0.96
        self._mci_wrapper.set_layer(3, 1.36, ua, us, g, d)

    def set_muscle_wavelength(self, wavelength):
        """shorter version of setting muscle layer"""
        ua = self.calc_ua_muscle(wavelength)
        us = self.calc_us_muscle(wavelength)
        self.set_muscle(ua, us)

    def calc_us_muscle(self, wavelength):
        # for us in the muscle layer we don't use mie theory but the
        # approximation presented in
        # Rowe et al.
        # "Modelling and validation of spectral reflectance for the colon"
        return 168.52 * (wavelength * 10 ** 9) ** -0.332 / (1. - 0.96) * 100.

    def calc_ua_muscle(self, wavelength):
        # for ua in the muscle layer we don't use mie theory but the
        # approximation presented in
        # Rowe et al.
        # "Modelling and validation of spectral reflectance for the colon"
        # calculated to retrieve an absorption of 11.2 cm-1 at 515nm
        ua = Ua()
        A = 1.7923385088285804
        ua.bvf = 0.1 * A
        ua.saO2 = 0.7
        ua.cHb = 120.
        return ua(wavelength)

    def __init__(self, params):
        self._mci_wrapper = MciWrapper()
        self.nr_photons = 10 ** 6
        self._mci_wrapper.add_layer(1.0, 0, 0, 1, 0.01)  # layer 0: air
        self._mci_wrapper.add_layer(1.38, 0, 0, 1, 0.05)  # layer 1: mucosa
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 0.05)  # layer 2: submucosa
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 0.09)  # layer 3: muscle
