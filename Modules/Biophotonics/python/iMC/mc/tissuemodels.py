'''
Created on Sep 9, 2015

@author: wirkert
'''

from mc.sim import MciWrapper
from mc.usuag import Ua, UsG


def calc_us_muscle(wavelength):
    """helper function for setting us in muscle layer.
    for us in the muscle layer we don't use mie theory but the
    approximation presented in
    Rowe et al.
    "Modelling and validation of spectral reflectance for the colon"
    """
    return 168.52 * (wavelength * 10 ** 9) ** -0.332 / (1. - 0.96) * 100.


def calc_ua_muscle(wavelength):
    """helper function for setting ua in muscle layer.
    for ua_sm in the muscle layer we don't use mie theory but the
    approximation presented in
    Rowe et al.
    "Modelling and validation of spectral reflectance for the colon"
    calculated to retrieve an absorption of 11.2 cm-1 at 515nm
    """
    ua = Ua()
    A = 1.7923385088285804
    ua.bvf = 0.1 * A
    ua.saO2 = 0.7
    ua.cHb = 120.
    return ua(wavelength)


class Colon(object):
    '''
    Initializes a three layer colon tissue model as e.g. used by
    Rowe et al. "Modelling and validation of spectral reflectance for the colon"
    '''

    def set_mci_filename(self, mci_filename):
        self._mci_wrapper.set_mci_filename(mci_filename)

    def set_mco_filename(self, mco_filename):
        self._mci_wrapper.set_mco_filename(mco_filename)

    def set_wavelength(self, wavelength):
        self.wavelength = wavelength

    def set_mucosa(self, bvf=None, saO2=None, dsp=None, r=None, d=None):
        if bvf is None:
            bvf = 0.02
        if saO2 is None:
            saO2 = 0.7
        if dsp is None:
            dsp = 0.015
        if d is None:
            d = 500. * 10 ** -6
        if r is None:
            r = 0.4 * 10.** -6
        # build obejct for absorption coefficient determination
        self.ua_muc.bvf = bvf
        self.ua_muc.saO2 = saO2
        # and one for scattering coefficient
        self.usg_muc.dsp = dsp
        self.usg_muc.r = r
        self.d_muc = d

    def set_submucosa(self, bvf=None, saO2=None, dsp=None, r=None, d=None):
        if bvf is None:
            bvf = 0.1
        if saO2 is None:
            saO2 = self._mucosa_saO2
        if dsp is None:
            dsp = 0.3
        if r is None:
            r = 2 * 10.** -6
        if d is None:
            d = 500. * 10 ** -6
        # build object for absorption coefficient determination
        self.ua_sm.bvf = bvf
        self.ua_sm.saO2 = saO2
        # and one for scattering coefficient
        self.usg_sm = UsG()
        self.usg_sm.dsp = dsp
        self.usg_sm.r = r
        # set the layer thickness
        self.d_sm = d

    def create_mci_file(self):
        # set additional values
        self._mci_wrapper.set_nr_photons(self.nr_photons)
        # calculate values for mucosa
        us_muc, g_muc = self.usg_muc(self.wavelength)
        self._mci_wrapper.set_layer(1, 1.38, self.ua_muc(self.wavelength),
                                    us_muc, g_muc, self.d_muc)
        # calculate values for submucosa
        us_sm, g_sm = self.usg_sm(self.wavelength)
        self._mci_wrapper.set_layer(2, 1.36, self.ua_sm (self.wavelength),
                                    us_sm, g_sm, self.d_sm)
        # shorter version for setting muscle layer
        ua_mus = calc_ua_muscle(self.wavelength)
        us_mus = calc_us_muscle(self.wavelength)
        self._mci_wrapper.set_layer(3, 1.36, ua_mus, us_mus,
                                    0.96, 900. * 10 ** -6)
        # now that the layers have been updated: create file
        self._mci_wrapper.create_mci_file()

    def __init__(self):
        self.ua_muc = Ua()
        self.usg_muc = UsG()
        self.ua_sm = Ua()
        self.usg_sm = UsG()
        self._mci_wrapper = MciWrapper()
        self.nr_photons = 10 ** 6
        self._mucosa_saO2 = 0.7
        # initially create layers. these will be overwritten as soon
        # as create_mci_file is called.
        self._mci_wrapper.add_layer(1.0, 0, 0, 1, 0.01)  # layer 0: air
        # layer 1: mucosa
        self._mci_wrapper.add_layer(1.38, 0, 0, 1, 500. * 10 ** -6)
        # layer 2: submucosa
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 500. * 10 ** -6)
        # layer 3: muscle
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 900. * 10 ** -6)
