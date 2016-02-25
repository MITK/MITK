# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 09:52:47 2015

@author: wirkert
"""

import unittest
import os
import numpy as np

import msi.msimanipulations as msimani
from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
from msi.test import helpers


class TestNrrdWriter(unittest.TestCase):

    def setUp(self):
        # setup file and the path where it shall be written to
        self.msi = helpers.getFakeMsi()
        self.fileUriToWrite = "testfile.nrrd"

    def tearDown(self):
        # remove the hopefully written file
        os.remove(self.fileUriToWrite)

    def test_imageWriterCreatesFile(self):
        writer = NrrdWriter(self.msi)
        writer.write(self.fileUriToWrite)
        self.assertTrue(os.path.isfile(self.fileUriToWrite),
                        "file was written to disk")

    def test_imageWriterCreatesCorrectFile(self):

        writer = NrrdWriter(self.msi)
        writer.write(self.fileUriToWrite)

        reader = NrrdReader()
        msi = reader.read(self.fileUriToWrite)
        self.assertTrue(msi == helpers.getFakeMsi(),
                                       "image correctly written and read")

    def test_write_one_d_image_works(self):
        writer = NrrdWriter(self.msi)
        msimani.calculate_mean_spectrum(self.msi)
        writer.write(self.fileUriToWrite)

        reader = NrrdReader()
        msi = reader.read(self.fileUriToWrite)
        np.testing.assert_array_equal(msi.get_image(),
                                      np.array([1, 2, 3, 4, 5]),
                                      "1d image correctly written and read")
