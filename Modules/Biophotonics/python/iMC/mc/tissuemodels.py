'''
Created on Sep 9, 2015

@author: wirkert
'''

import numpy as np

from mc.sim import MciWrapper
from mc.usuag import Ua, UsgMie, UsgJacques, UaMuscle, UsGMuscle


class AbstractTissue(object):
    '''
    Initializes a three layer colon tissue model"
    '''

    def set_mci_filename(self, mci_filename):
        self._mci_wrapper.set_mci_filename(mci_filename)

    def set_mco_filename(self, mco_filename):
        self._mci_wrapper.set_mco_filename(mco_filename)

    def set_wavelength(self, wavelength):
        self.wavelength = wavelength

    def create_mci_file(self):
        # set additional values
        self._mci_wrapper.set_nr_photons(self.nr_photons)
        # calculate values for layer 1; e.g. mucosa
        us_l1, g_l1 = self.usg_l1(self.wavelength)
        self._mci_wrapper.set_layer(1, 1.38, self.ua_l1(self.wavelength),
                                    us_l1, g_l1, self.d_l1)
        # calculate values for layer 2; e.g. submucosa
        us_l2, g_l2 = self.usg_l2(self.wavelength)
        self._mci_wrapper.set_layer(2, 1.36, self.ua_l2 (self.wavelength),
                                    us_l2, g_l2, self.d_l2)
        # shorter version for setting layer 3; e.g. muscle layer
        ua_l3 = self.ua_l3(self.wavelength)
        us_l3, g_l3 = self.usg_l3(self.wavelength)
        self._mci_wrapper.set_layer(3, 1.36, ua_l3, us_l3,
                                    g_l3, self.d_l3)
        # now that the layers have been updated: create file
        self._mci_wrapper.create_mci_file()

    def get_layer_parameters(self):
        """ Overwrite this method!
        get the model parameters as two numpy arrays, one for each layer"""
        layer1 = np.array([])
        layer2 = np.array([])
        layer3 = np.array([])
        return layer1, layer2, layer3

    def __str__(self):
        """ Overwrite this method!
        print the current model"""
        model_string = \
                    ""
        return model_string

    def __init__(self, ua_muc, usg_muc, ua_sm, usg_sm, ua_l3, usg_l3):
        self._mci_wrapper = MciWrapper()
        self.nr_photons = 10 ** 6
        # layer 1:
        self.ua_l1 = ua_muc
        self.usg_l1 = usg_muc
        self.d_l1 = 500.*10 ** -6
        # layer 2:
        self.ua_l2 = ua_sm
        self.usg_l2 = usg_sm
        self.d_l2 = 500.*10 ** -6
        # layer 3:
        self.ua_l3 = ua_l3
        self.usg_l3 = usg_l3
        self.d_l3 = 900. * 10 ** -6

        self._mci_wrapper = MciWrapper()
        self.nr_photons = 10 ** 6
        self._mucosa_saO2 = 0.7
        # initially create layers. these will be overwritten as soon
        # as create_mci_file is called.
        self._mci_wrapper.add_layer(1.0, 0, 0, 1, 0.01)  # layer 0: air
        # layer 1: e.g. mucosa
        self._mci_wrapper.add_layer(1.38, 0, 0, 1, 500. * 10 ** -6)
        # layer 2: e.g. submucosa
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 500. * 10 ** -6)
        # layer 3: e.g muscle
        self._mci_wrapper.add_layer(1.36, 0, 0, 1, 900. * 10 ** -6)


class ColonRowe(AbstractTissue):
    '''
    Initializes a three layer colon tissue model as e.g. used by
    Rowe et al. "Modelling and validation of spectral reflectance for the colon"
    '''

    def set_mucosa(self, bvf=None, saO2=None, cBili=None,
                   dsp=None, r=None, d=None):
        if bvf is None:
            bvf = 0.02
        if saO2 is None:
            saO2 = 0.7
        if cBili is None:
            cBili = 0.0123
        if dsp is None:
            dsp = 0.015
        if d is None:
            d = 500. * 10 ** -6
        if r is None:
            r = 0.4 * 10.** -6 / 2.
        # build obejct for absorption coefficient determination
        self.ua_l1.bvf = bvf
        self.ua_l1.saO2 = saO2
        self.ua_l1.cBili = cBili
        # and one for scattering coefficient
        self.usg_l1.dsp = dsp
        self.usg_l1.r = r
        self.d_l1 = d

    def set_submucosa(self, bvf=None, saO2=None, dsp=None, r=None, d=None):
        if bvf is None:
            bvf = 0.1
        if saO2 is None:
            saO2 = self.ua_l1.saO2
        if dsp is None:
            dsp = 0.3
        if r is None:
            r = 2 * 10.** -6 / 2.
        if d is None:
            d = 500. * 10 ** -6
        # build object for absorption coefficient determination
        self.ua_l2.bvf = bvf
        self.ua_l2.saO2 = saO2
        # and one for scattering coefficient
        self.usg_l2 = UsgMie()
        self.usg_l2.dsp = dsp
        self.usg_l2.r = r
        # set the layer thickness
        self.d_l2 = d

    def get_layer_parameters(self):
        """get the model parameters as two numpy arrays, one for mucosa and one
        for submucusa"""
        mucosa = np.array([self.ua_l1.bvf, self.ua_l1.saO2, self.ua.muc.cBili,
                         self.usg_l1.dsp, self.usg_l1.r,
                         self.d_l1])
        submucosa = np.array([self.ua_l2.bvf, self.ua_l2.saO2,
                         self.usg_l2.dsp, self.usg_l2.r,
                         self.d_l2])
        return mucosa, submucosa

    def __str__(self):
        """print the current model"""
        model_string = \
                    "mucosa    - bvf: " + "%.2f" % self.ua_l1.bvf + \
                    "%; SaO2: " + "%.2f" % self.ua_l1.saO2 + \
                    "%; bilirubin: " + "%.1f" % (self.ua_l1.cBili * 100) + \
                    "ug/dl; dsp: " + "%.2f" % self.usg_l1.dsp + \
                    "%; r: " + "%.2f" % (self.usg_l1.r * 10 ** 6) + \
                    "um; d: " + "%.0f" % (self.d_l1 * 10 ** 6) + "um\n" + \
                    "submucosa - bvf: " + "%.2f" % (self.ua_l2.bvf) + \
                    "%; SaO2: " + "%.2f" % (self.ua_l2.saO2) + \
                    "%; dsp: " + "%.2f" % (self.usg_l2.dsp) + \
                    "%; r: " + "%.2f" % (self.usg_l2.r * 10 ** 6) + \
                    "um; d: " + "%.0f" % (self.d_l2 * 10 ** 6) + "um"
        return model_string

    def __init__(self):
        super(ColonRowe, self).__init__(Ua(), UsgMie(), Ua(), UsgMie(),
                                        UaMuscle(), UsGMuscle())



class ColonJacques(AbstractTissue):
    '''
    Initializes a three layer colon tissue model
    '''

    def set_mucosa(self, bvf=None, saO2=None, cBili=None, a_mie=None, a_ray=None, d=None):
        if bvf is None:
            bvf = 0.02
        if saO2 is None:
            saO2 = 0.7
        if cBili is None:
            cBili = 0.0123
        if a_mie is None:
            a_mie = 10. * 100
        if d is None:
            d = 500. * 10 ** -6
        if a_ray is None:
            a_ray = 0.
        # build obejct for absorption coefficient determination
        self.ua_l1.bvf = bvf
        self.ua_l1.saO2 = saO2
        self.ua_l1.cBili = cBili
        # and one for scattering coefficient
        self.usg_l1.a_mie = a_mie
        self.usg_l1.a_ray = a_ray
        self.d_l1 = d

    def set_submucosa(self, bvf=None, saO2=None, dsp=None, r=None, d=None):
        if bvf is None:
            bvf = 0.1
        if saO2 is None:
            saO2 = self.ua_l1.saO2
        if dsp is None:
            dsp = 0.3
        if r is None:
            r = 2 * 10.** -6 / 2.
        if d is None:
            d = 500. * 10 ** -6
        # build object for absorption coefficient determination
        self.ua_l2.bvf = bvf
        self.ua_l2.saO2 = saO2
        # and one for scattering coefficient
        self.usg_l2.dsp = dsp
        self.usg_l2.r = r
        # set the layer thickness
        self.d_l2 = d

    def get_layer_parameters(self):
        """get the model parameters as two numpy arrays, one for mucosa and one
        for submucusa"""
        mucosa = np.array([self.ua_l1.bvf, self.ua_l1.saO2, self.ua_l1.cBili,
                         self.usg_l1.a_mie, self.usg_l1.a_ray,
                         self.usg_l1.b_mie,
                         self.d_l1])
        submucosa = np.array([self.ua_l2.bvf, self.ua_l2.saO2,
                         self.usg_l2.dsp, self.usg_l2.r,
                         self.d_l2])
        return mucosa, submucosa

    def __str__(self):
        """print the current model"""
        model_string = \
                    "mucosa    - bvf: " + "%.2f" % self.ua_l1.bvf + \
                    "%; SaO2: " + "%.2f" % self.ua_l1.saO2 + \
                    "%; bilirubin: " + "%.1f" % (self.ua_l1.cBili * 100) + \
                    "mg/dl; a_mie: " + "%.2f" % (self.usg_l1.a_mie / 100.) + \
                    "cm^-1; a_ray: " + "%.2f" % (self.usg_l1.a_ray / 100.) + \
                    "cm^-1; b_mie: " + "%.3f" % self.usg_l1.b_mie + \
                    "; d: " + "%.0f" % (self.d_l1 * 10 ** 6) + "um\n" + \
                    "submucosa - bvf: " + "%.2f" % (self.ua_l2.bvf) + \
                    "%; SaO2: " + "%.2f" % (self.ua_l2.saO2) + \
                    "%; dsp: " + "%.2f" % (self.usg_l2.dsp) + \
                    "%; r: " + "%.2f" % (self.usg_l2.r * 10 ** 6) + \
                    "um; d: " + "%.0f" % (self.d_l2 * 10 ** 6) + "um"
        return model_string

    def __init__(self):
        super(ColonJacques, self).__init__(Ua(), UsgJacques(), Ua(), UsgMie(),
                                           UaMuscle(), UsGMuscle())



class GenericTissue(AbstractTissue):
    '''
    Initializes a three layer generic tissue model
    '''

    def set_layer1(self, bvf=None, saO2=None, a_mie=None, a_ray=None, d=None):
        if bvf is None:
            bvf = 0.02
        if saO2 is None:
            saO2 = 0.7
        if a_mie is None:
            a_mie = 10. * 100
        if d is None:
            d = 500. * 10 ** -6
        if a_ray is None:
            a_ray = 0.
        # build obejct for absorption coefficient determination
        self.ua_l1.bvf = bvf
        self.ua_l1.saO2 = saO2
        # and one for scattering coefficient
        self.usg_l1.a_mie = a_mie
        self.usg_l1.a_ray = a_ray
        self.d_l1 = d

    def set_layer2(self, bvf=None, saO2=None, a_mie=None, a_ray=None, d=None):
        if bvf is None:
            bvf = 0.1
        if saO2 is None:
            saO2 = self.ua_l1.saO2
        if a_mie is None:
            a_mie = 10. * 100
        if a_ray is None:
            a_ray = 0.
        if d is None:
            d = 500. * 10 ** -6
        # build object for absorption coefficient determination
        self.ua_l2.bvf = bvf
        self.ua_l2.saO2 = saO2
        # and one for scattering coefficient
        self.usg_l2.a_mie = a_mie
        self.usg_l2.a_ray = a_ray
        # set the layer thickness
        self.d_l2 = d


    def set_layer3(self, bvf=None, saO2=None, a_mie=None, a_ray=None, d=None):
        if bvf is None:
            bvf = 0.1
        if saO2 is None:
            saO2 = self.ua_l2.saO2
        if a_mie is None:
            a_mie = 10. * 100
        if a_ray is None:
            a_ray = 0.
        if d is None:
            d = 500. * 10 ** -6
        # build object for absorption coefficient determination
        self.ua_l3.bvf = bvf
        self.ua_l3.saO2 = saO2
        # and one for scattering coefficient
        self.usg_l3.a_mie = a_mie
        self.usg_l3.a_ray = a_ray
        # set the layer thickness
        self.d_l3 = d

    def get_layer_parameters(self):
        """get the model parameters as three numpy arrays, one for each layer"""
        layer1 = np.array([self.ua_l1.bvf, self.ua_l1.saO2, self.ua_l1.cBili,
                         self.usg_l1.a_mie, self.usg_l1.a_ray,
                         self.usg_l1.b_mie,
                         self.d_l1])
        layer2 = np.array([self.ua_l2.bvf, self.ua_l2.saO2,
                         self.usg_l2.dsp, self.usg_l2.r,
                         self.d_l2])
        layer3 = np.array([self.ua_l3.bvf, self.ua_l3.saO2,
                         self.usg_l23.dsp, self.usg_l3.r,
                         self.d_l3])
        return layer1, layer2, layer3

    def __str__(self):
        """print the current model"""
        model_string = \
                    "layer1    - bvf: " + "%.2f" % self.ua_l1.bvf + \
                    "%; SaO2: " + "%.2f" % self.ua_l1.saO2 + \
                    "mg/dl; a_mie: " + "%.2f" % (self.usg_l1.a_mie / 100.) + \
                    "cm^-1; a_ray: " + "%.2f" % (self.usg_l1.a_ray / 100.) + \
                    "cm^-1; b_mie: " + "%.3f" % self.usg_l1.b_mie + \
                    "; d: " + "%.0f" % (self.d_l1 * 10 ** 6) + "um\n" + \
                    "layer2    - bvf: " + "%.2f" % self.ua_l2.bvf + \
                    "%; SaO2: " + "%.2f" % self.ua_l2.saO2 + \
                    "mg/dl; a_mie: " + "%.2f" % (self.usg_l2.a_mie / 100.) + \
                    "cm^-1; a_ray: " + "%.2f" % (self.usg_l2.a_ray / 100.) + \
                    "cm^-1; b_mie: " + "%.3f" % self.usg_l2.b_mie + \
                    "; d: " + "%.0f" % (self.d_l1 * 10 ** 6) + "um\n" + \
                                        "layer1    - bvf: " + "%.2f" % self.ua_l1.bvf + \
                    "%; SaO2: " + "%.2f" % self.ua_l3.saO2 + \
                    "mg/dl; a_mie: " + "%.2f" % (self.usg_l3.a_mie / 100.) + \
                    "cm^-1; a_ray: " + "%.2f" % (self.usg_l3.a_ray / 100.) + \
                    "cm^-1; b_mie: " + "%.3f" % self.usg_l3.b_mie + \
                    "; d: " + "%.0f" % (self.d_l3 * 10 ** 6) + "um\n"
        return model_string

    def __init__(self):
        super(AbstractTissue, self).__init__(Ua(), UsgJacques(),
                                             Ua(), UsgJacques(),
                                             Ua(), UsgJacques())

