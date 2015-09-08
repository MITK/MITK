# -*- coding: utf-8 -*-
"""
Created on Thu Aug  6 18:21:36 2015

@author: wirkert
"""

import numpy as np

class Msi():
    """ a multi spectral image stack consisting of:

    image:      a columns x rows x nrWavelengths dimensional array
    properties: additional, application dependent properties
    """

    def __init__(self, image=None, properties=None):
        if image is None:
            image = np.array([])
        if properties is None:
            properties = {}
        self._image = image
        self._properties = properties

    def get_image(self):
        return self._image

    def set_image(self, image):
        self._image = image

    def get_wavelengths(self):
        """ shortcut to get the wavelengths property
        The wavelengths are given in [m] units and need not be sorted. """
        if ('wavelengths' not in self._properties):
            return None
        return self._properties['wavelengths']

    def set_wavelengths(self, wavelengths):
        """ shortcut to set the wavelengths property """
        self._properties['wavelengths'] = wavelengths

    def get_properties(self):
        return self._properties

    def add_property(self, newProperty):
        """ add a new property(ies) to the existing properties """
        self._properties.update(newProperty)

    def set_mask(self, mask):
        """" applies a masked to the Msi. After this call, the image is of
        type MaskedArray. If the image was already masked, the existing
        masked will be "or ed" with the new mask. mask is a boolean array of
        the same shape as self.get_image()"""


        if not isinstance(self.get_image(), np.ma.MaskedArray):
            self.set_image(np.ma.masked_array(self.get_image(), mask,
                          fill_value=1e+20))
        else:
            self.get_image()[mask] = np.ma.masked

    def __eq__(self, other):
        """
        overrite the == operator
        Two Msi s are the same if they contain the same image and properties.
        Note: properties not implemented yet!
        """
        if isinstance(other, Msi):
            samesame = np.array_equal(other.get_image(), self.get_image())
            return samesame

        return NotImplemented

    def __ne__(self, other):
        """ != operator implemented by inverting to =="""
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

