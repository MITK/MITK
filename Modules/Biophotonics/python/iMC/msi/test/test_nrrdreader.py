# -*- coding: utf-8 -*-
"""
Created on Mon Aug 10 16:43:31 2015

@author: wirkert
"""

import unittest
from msi.io.nrrdreader import NrrdReader
import numpy as np


class TestNrrdReader(unittest.TestCase):

    def setUp(self):
        self.nrrdReader = NrrdReader()
        self.msi = self.nrrdReader.read('./msi/data/testMsi.nrrd')

    def test_read_does_not_crash(self):
        # if we got this far, at least an image was read.
        self.assertTrue(len(self.msi.get_image().shape) == 3,
                        "read image has correct basic shape dimensions")
        self.assertTrue(self.msi.get_image().shape[-1] == 5,
                        "read image has correct number of image stacks")
        self.assertTrue(np.array_equal(self.msi.get_image()[2, 2, :],
                        np.array([1, 2, 3, 4, 5])),
                        "read image contains correct data")

    def test_read_non_existing_image_returns_exception(self):
        with self.assertRaises(RuntimeError):
            self.nrrdReader.read("./msi/data/asdf.nrrd")
