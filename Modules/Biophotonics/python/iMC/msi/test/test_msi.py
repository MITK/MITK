# -*- coding: utf-8 -*-
"""
Created on Fri Mar 27 19:12:40 2015

@author: wirkert
"""

import unittest
import numpy as np

from msi.msi import Msi
from msi.test import helpers


class TestMsi(unittest.TestCase):

    def setUp(self):
        self.msi = helpers.getFakeMsi()

    def tearDown(self):
        pass

    def test_create(self):
        self.assertTrue(True, "Created msi during setup")

    def test_add_property(self):
        self.msi.add_property({'test':np.array([1, 2, 3])})
        self.assertTrue(np.array_equal(self.msi.get_properties()['test'],
            np.array([1, 2, 3])), "property successfully added to msi")

    def test_properties_not_shared(self):
        msi1 = Msi()
        msi2 = Msi()
        msi1.add_property({"integration time": np.array([1, 2, 3])})

        self.assertTrue('integration time' not in msi2.get_properties())

    def test_add_dummy_wavelengths_automatically(self):
        msi_no_wavelengths_set = Msi()
        msi_no_wavelengths_set.set_image(self.msi.get_image())

        nr_wavelengths = msi_no_wavelengths_set.get_image().shape[-1]

        np.testing.assert_equal(msi_no_wavelengths_set.get_wavelengths(),
                                np.arange(nr_wavelengths),
                                "correct dummy wavelength values set")




