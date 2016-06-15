# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 13:52:23 2015

@author: wirkert
"""


import unittest
import copy
import numpy as np

from msi.test import helpers
import msi.msimanipulations as mani


class TestMsiManipulations(unittest.TestCase):

    def setUp(self):
        self.msi = helpers.getFakeMsi()
        self.specialmsi = helpers.getFakeMsi()

        # set one pixel to special values
        self.specialValue = np.arange(self.specialmsi.get_image().shape[-1]) * 2
        self.specialmsi.get_image()[2, 2, :] = self.specialValue

        # create a segmentation which sets all elements to invalid but the
        # one pixel with the special value
        self.segmentation = np.zeros(self.specialmsi.get_image().shape[0:-1])
        self.segmentation[2, 2] = 1

    def tearDown(self):
        pass

    def test_apply_segmentation(self):

        mani.apply_segmentation(self.specialmsi, self.segmentation)

        validImageEntries = self.specialmsi.get_image() \
            [~self.specialmsi.get_image().mask]

        np.testing.assert_equal(validImageEntries, self.specialValue,
                        "image has been correctly segmented")

    def test_calculate_mean_spectrum(self):

        mani.calculate_mean_spectrum(self.specialmsi)

        np.testing.assert_equal(np.array([0.96, 2., 3.04, 4.08, 5.12]),
                                self.specialmsi.get_image(),
                        "mean spectrum is correctly calculated on image with " +
                        "no mask applied")

    def test_calculate_mean_spectrum_masked_image(self):

        mani.apply_segmentation(self.specialmsi, self.segmentation)
        mani.calculate_mean_spectrum(self.specialmsi)

        np.testing.assert_equal(self.specialValue, self.specialmsi.get_image(),
                        "mean spectrum is correctly calculated on image with " +
                        "mask applied")

    def test_interpolate(self):
        # create not sorted new wavelengths
        newWavelengths = np.array([4.0, 2.5, 3.5, 1.5])
        mani.interpolate_wavelengths(self.msi, newWavelengths)

        np.testing.assert_equal(newWavelengths, self.msi.get_wavelengths(),
                        "wavelengths correctly updated")
        # check if first image pixel was correctly calculated
        # (hopefully true for all then)
        np.testing.assert_equal(np.array([2.0, 3.5, 2.5, 4.5]),
                                self.msi.get_image()[0, 0, :],
                        "image elements correctly interpolated")

    def test_normalize_integration_times(self):
        old_shape = self.msi.get_image().shape
        integration_times = np.array([1., 2., 3., 4., 5.])
        self.msi.add_property({'integration times': integration_times})
        mani.normalize_integration_times(self.msi)

        np.testing.assert_equal(self.msi.get_image()[1, 3, :],
                            np.ones_like(integration_times),
                            "normalized integration times")
        np.testing.assert_equal(self.msi.get_properties()['integration times'],
                            np.ones_like(integration_times),
                            "integration time property set to ones")
        self.assertEqual(self.msi.get_image().shape, old_shape,
                        "shape did not change from normalizing")

    def test_normalize_integration_times_none_given(self):
        msi_copy = copy.deepcopy(self.msi)
        mani.normalize_integration_times(msi_copy)
        np.testing.assert_equal(msi_copy.get_image(), self.msi.get_image(),
                                "nothing change by normalizing without" + \
                                "integration times given")

    def test_dark_correction(self):
        desired_image_data = copy.copy(self.msi.get_image())
        desired_image_data -= 1

        dark = copy.copy(self.msi)
        dark.set_image(np.ones_like(dark.get_image()))

        mani.dark_correction(self.msi, dark)

        np.testing.assert_equal(self.msi.get_image(),
                                       desired_image_data,
                                       "dark image correctly accounted for")
        np.testing.assert_equal(dark.get_image(),
                                       np.ones_like(dark.get_image()),
                        "dark image unchanged by dark correction")

    def test_dark_correction_with_single_value(self):
        desired_image_data = copy.copy(self.specialmsi.get_image())
        desired_image_data -= 1

        dark = copy.copy(self.specialmsi)
        dark.set_image(np.ones_like(dark.get_image()))
        mani.calculate_mean_spectrum(dark)

        mani.dark_correction(self.specialmsi, dark)

        np.testing.assert_equal(self.specialmsi.get_image(),
                                       desired_image_data,
                "dark image correctly accounted for from singular dark value")
        np.testing.assert_equal(dark.get_image(),
                                       np.ones_like(dark.get_image()),
                "dark image unchanged by dark correction")

    def test_flatfield_correction(self):
        desired_image_data = np.ones_like(self.specialmsi.get_image())
        desired_image_data[2, 2, 0] = np.nan

        mani.flatfield_correction(self.specialmsi, self.specialmsi)

        np.testing.assert_equal(self.specialmsi.get_image(),
                                       desired_image_data,
                       "correct image by itself should lead to only 1s ")

    def test_flatfield_correction_differing_integration_times(self):
        MSI_INTEGRATION_TIME = 3.0
        FLATFIELD_INTEGRATION_TIME = 2.0
        desired_image_data = np.ones_like(self.specialmsi.get_image()) * \
             FLATFIELD_INTEGRATION_TIME / MSI_INTEGRATION_TIME
        desired_image_data[2, 2, 0] = np.nan
        self.specialmsi.add_property({"integration times":
                                      np.ones_like(
                                        self.specialmsi.get_image()[0, 0, :])
                                        * MSI_INTEGRATION_TIME})
        flatfield = copy.deepcopy(self.specialmsi)
        flatfield.add_property({"integration times":
                                np.ones_like(
                                    flatfield.get_image()[0, 0, :])
                                    * FLATFIELD_INTEGRATION_TIME})
        # for testing if flatfield does not changed by correction we copy it
        flatfield_copy = copy.deepcopy(flatfield)

        mani.flatfield_correction(self.specialmsi, flatfield_copy)

        np.testing.assert_almost_equal(self.specialmsi.get_image(),
                                       desired_image_data, 15,
                       "corrected image is a division of integration times")
        np.testing.assert_equal(flatfield.get_image(),
                                flatfield_copy.get_image(),
                                "flatfield doesn't change by correction")

    def test_flatfield_correction_with_single_value(self):
        desired_image_data = np.ones_like(self.msi.get_image())
        flatfield = copy.copy(self.msi)
        mani.calculate_mean_spectrum(flatfield)
        unchanged_flatfield = copy.deepcopy(flatfield)

        mani.flatfield_correction(self.msi, flatfield)

        np.testing.assert_equal(self.msi.get_image(),
                                       desired_image_data,
                "flatfield correctly accounted for from singular reference value")
        np.testing.assert_equal(flatfield, unchanged_flatfield,
                "flatfield not changed by algorithm")

    def test_image_correction(self):
        dark = copy.copy(self.msi)
        dark.set_image(np.ones_like(dark.get_image()) * 0.5)
        flatfield = copy.copy(self.msi)
        flatfield_copy = copy.deepcopy(flatfield)
        dark_copy = copy.deepcopy(dark)

        mani.image_correction(self.msi, flatfield, dark)

        np.testing.assert_equal(flatfield.get_image(),
                                flatfield_copy.get_image(),
                                "image correction didn't change flatfield")
        np.testing.assert_equal(dark.get_image(), dark_copy.get_image(),
                                "image correction didn't change dark image")
        np.testing.assert_almost_equal(self.msi.get_image(),
                                       np.ones_like(self.msi.get_image()),
                                       15, "image correctly corrected :-)")





