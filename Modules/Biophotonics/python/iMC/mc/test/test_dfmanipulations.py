'''
Created on Oct 19, 2015

@author: wirkert
'''
import unittest

import numpy as np
from pandas.util.testing import assert_frame_equal

from mc.batches import ColonMuscleBatch
import mc.dfmanipulations as dfmani

class Test(unittest.TestCase):

    def setUp(self):
        # create a colon batch with 2 samples
        self.test_batch = ColonMuscleBatch()
        self.test_batch.create_parameters(2)

        # artificially add 10 fake "reflectances" to this batch
        # at 10 fake "wavelengths"
        WAVELENGHTS = np.linspace(450, 720, 10)
        reflectance1 = np.arange(0, 30, 3)
        reflectance2 = np.arange(30, 60, 3)
        for w in WAVELENGHTS:
            self.test_batch.df["reflectances", w] = np.NAN
        for r1, r2, w in zip(reflectance1, reflectance2, WAVELENGHTS):
            self.test_batch.df["reflectances", w][0] = r1
            self.test_batch.df["reflectances", w][1] = r2

        # shortcut to dataframe that we are interested in:
        self.df = self.test_batch.df

    def test_sliding_average(self):
        # by test design folding should not alter elements (only at boundaries,
        # which are excluded by array slicing:
        expected_elements = self.df.reflectances.iloc[:, 1:-1].copy()
        dfmani.fold_by_sliding_average(self.df, 3)

        assert_frame_equal(self.df.reflectances, expected_elements)

    def test_interpolation(self):
        new_wavelengths = [465, 615, 555]

        dfmani.interpolate_wavelengths(self.df, new_wavelengths)

        expected = np.array([[1.5, 16.5, 10.5], [31.5, 46.5, 40.5]])
        np.testing.assert_almost_equal(self.df.reflectances.as_matrix(),
                                       expected,
                                       err_msg="test if interpolation " +
                                       "works fine on batches")

    def test_select_n(self):
        """ this is less a test and more a showing of how to select n elements
            from a dataframe."""
        # draw one sample. Look into documentation for sample to see all the
        # options. Sample is quite powerfull.
        self.df = self.df.sample(1)
        self.assertEqual(self.df.shape[0], 1,
                         "one sample selected")

    def test_sortout_bands(self):
        """ this is less a test and more a showing of how to sortout specific
            bands from a dataframe """
        # drop the 510 and 720 nm band
        band_names_to_sortout = [510, 720]
        self.df.drop(band_names_to_sortout, axis=1, level=1, inplace=True)

        df_r = self.df["reflectances"]
        self.assertTrue(not (510 in df_r.columns))
        self.assertTrue(not 720 in df_r.columns)
        self.assertTrue(690 in df_r.columns)


if __name__ == "__main__":
    # import sys;sys.argv = ['', 'Test.testName']
    unittest.main()
