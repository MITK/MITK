# -*- coding: utf-8 -*-
"""
Created on Thu Aug  6 18:21:36 2015

@author: wirkert
"""

import numpy as np


class Msi():
    """ a multi spectral image stack consisting of:

    image:      a rows x columns x nrWavelengths dimensional array
    properties: additional, application dependent properties
    """

    def __init__(self, image=None, properties=None):
        if image is None:
            image = np.array([])
        if properties is None:
            properties = {}
        self._image = image
        self._properties = properties
        self._assure_basic_properties()

        self._test_image()

    def get_image(self):
        return self._image

    def set_image(self, image, wavelengths=None):
        """
        Put a new image into this msi
        Args:
            image: the rows x columns x nrWavelengths dimensional array
                   np.array.
            wavelengths: a np.array of size nrWavelengths. If the number of
                         wavelengths hasn't change this is not needed.
        """
        self._image = image
        if wavelengths is not None:
            self.set_wavelengths(wavelengths)
        self._assure_basic_properties()
        self._test_image()

    def get_wavelengths(self):
        """ shortcut to get the wavelengths property
        The wavelengths are given in [m] units and need not be sorted. """
        if 'wavelengths' not in self.get_properties():
            return None
        return self._properties['wavelengths']

    def set_wavelengths(self, wavelengths):
        """ shortcut to set the wavelengths property """
        w_prop = {"wavelengths":wavelengths}
        self.add_property(w_prop)
        self._test_image()

    def get_properties(self):
        return self._properties

    def add_property(self, newProperty):
        """ add a new property(ies) to the existing properties """
        self._properties.update(newProperty)
        self._test_image()

    def set_mask(self, mask):
        """" applies a masked to the Msi. After this call, the image is of
        type MaskedArray. If the image was already masked, the existing
        masked will be "or ed" with the new mask. mask is a boolean array of
        the same shape as self.get_image()

        Args:
            mask: a mask of the same size as the image. 1s stand for pixels
                  masked out, 0s for pixels not masked."""
        if not isinstance(self.get_image(), np.ma.MaskedArray):
            self.set_image(np.ma.masked_array(self.get_image(), mask,
                           fill_value=999999))
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

    def _assure_basic_properties(self):
        """
        helper method to automatically add the basic properties:
        wavelength
        to the msi if not added explicicly. basic wavelengths will just be
        integers from 0 to 1
        """
        if self._image.size > 0 and (
                ("wavelengths" not in self._properties.keys() or
                self._properties["wavelengths"].size == 0)):
            self._properties["wavelengths"] = np.arange(self._image.shape[-1])
        if self._image.size == 0 and "wavelengths" not in self._properties.keys():
            self._properties["wavelengths"] = np.array([])

    def _test_image(self):
        """
        helper method which tests for the integrity of the msi.
        E.g. the number of wavelengths must match the number of bands.
        """
        # either both image and wavelength property are empty
        if self._image.size == 0 and len(self._properties["wavelengths"]) != 0:
            raise RuntimeError("dimension of image and wavelength mismatch: " +
                               "image size is zero, but wavelengths are set")
        # or both are same
        elif self._image.shape[-1] != len(self._properties["wavelengths"]):
            raise RuntimeError("dimension of image and wavelength mismatch: " +
                               "image size and wavelenths do not match")

