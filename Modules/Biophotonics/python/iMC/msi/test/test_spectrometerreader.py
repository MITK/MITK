# -*- coding: utf-8 -*-
"""
Created on Fri Aug  7 18:02:08 2015

@author: wirkert
"""

import unittest

from msi.io.spectrometerreader import SpectrometerReader


class TestSpectrometer(unittest.TestCase):

    def setUp(self):
        self.exampleFileName = "./msi/data/Transmission_15-49-35-978_filter700nm.txt"
        self.reader = SpectrometerReader()

    def tearDown(self):
        pass

    def test_create(self):
        self.assertTrue(True, "Created empty reader during setup")

    def test_read_spectrum(self):
        msi = self.reader.read(self.exampleFileName)

        self.assertAlmostEqual(msi.get_image()[0],
                               70.50,
                               msg="first spectral element is read correctly")
        self.assertAlmostEqual(msi.get_image()[-1],
                               68.13,
                               msg="last sprectral element is read correctly")
        self.assertTrue(msi.get_image().size == 2048,
                               "correct number of elements read")

    def test_read_wavelengths(self):
        msi = self.reader.read(self.exampleFileName)

        self.assertAlmostEqual(msi.get_wavelengths()[0],
                               187.255 * 10 ** -9,
                               msg="first wavelength element is read correctly")
        self.assertAlmostEqual(msi.get_wavelengths()[-1],
                               1103.852 * 10 ** -9,
                               msg="last wavelength element is read correctly")
        self.assertTrue(msi.get_wavelengths().size == 2048,
                               "correct number of elements read")
