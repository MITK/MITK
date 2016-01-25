'''
Created on Aug 25, 2015

@author: wirkert
'''
import unittest
import os
import numpy as np

import msi.test.helpers as helpers
from msi.io.msiwriter import MsiWriter
from msi.io.msireader import MsiReader


class Test(unittest.TestCase):

    def setUp(self):
        self.msi = helpers.getFakeMsi()
        self.test_file_path = "test_msi.msi"

    def tearDown(self):
        # remove the hopefully written file
        os.remove(self.test_file_path)

    def test_read_and_write(self):
        reader = MsiReader()
        writer = MsiWriter(self.msi)
        writer.write(self.test_file_path)
        read_msi = reader.read(self.test_file_path)

        np.testing.assert_array_equal(self.msi.get_image(),
                                      read_msi.get_image(),
                                      "data array of msi stays same" +
                                      "after read and write")
        np.testing.assert_array_equal(
                        self.msi.get_properties()["wavelengths"],
                        read_msi.get_properties()["wavelengths"],
                        "properties of msi stay same after read and write")


