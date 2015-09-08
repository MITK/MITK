'''
Created on Aug 20, 2015

@author: wirkert
'''
import unittest
import numpy as np

import msi.normalize as norm
import helpers

class TestNormalize(unittest.TestCase):


    def setUp(self):
        self.specialmsi = helpers.getFakeMsi()
        # set one pixel to special values
        self.specialValue = np.arange(self.specialmsi.get_image().shape[-1]) * 2
        self.specialmsi.get_image()[2, 2, :] = self.specialValue

    def tearDown(self):
        pass


    def test_normalizeIQ(self):
        original_shape = self.specialmsi.get_image().shape  # shape should stay
        # value 4.0 is in band 3
        desired_matrix = self.specialmsi.get_image() / 4.0
        # except for special value, where it is 8
        desired_matrix[2, 2, :] = self.specialmsi.get_image()[2, 2, :] / 6.0
        # the same after normalization
        iq_normalizer = norm.NormalizeIQ(3)
        iq_normalizer.normalize(self.specialmsi)

        self.assertEqual(self.specialmsi.get_image().shape, original_shape,
                         "shape not changed by normalization")
        np.testing.assert_equal(self.specialmsi.get_image(),
                                desired_matrix,
                                "msi correctly normalized by iq")
