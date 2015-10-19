'''
Created on Oct 19, 2015

@author: wirkert
'''
import unittest

import numpy as np

from mc.batches import AbstractBatch
import mc.mcmanipulations as mcmani

class Test(unittest.TestCase):

    def setUp(self):
        self.test_batch = AbstractBatch()
        a = np.arange(0, 30, 3)
        b = np.arange(30, 60, 3)
        self.test_batch.reflectances = np.vstack((a, b))
        self.test_batch.wavelengths = np.arange(0, 10, 1)

    def test_sliding_average(self):
        # by design folding should not alter elements (only at boundaries,
        # which are excluded by array slicing:
        expected_elements = self.test_batch.reflectances[:, 1:-2]
        mcmani.fold_by_sliding_average(self.test_batch, 3)
        np.testing.assert_almost_equal(self.test_batch.reflectances[:, 1:-2],
                                       expected_elements,
                                       err_msg="test if sliding average " +
                                       "function works on batches")

    def test_interpolation(self):
        mcmani.interpolate_wavelengths(self.test_batch,
                                       np.array([0.5, 5.5, 3.5]))
        expected = np.array([[1.5, 16.5, 10.5], [31.5, 46.5, 40.5]])
        np.testing.assert_almost_equal(self.test_batch.reflectances, expected,
                                       err_msg="test if interpolation " +
                                       "works fine on batches")


if __name__ == "__main__":
    # import sys;sys.argv = ['', 'Test.testName']
    unittest.main()
