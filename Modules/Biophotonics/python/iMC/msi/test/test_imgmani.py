'''
Created on Aug 28, 2015

@author: wirkert
'''
import unittest
import numpy as np

import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.test.helpers as helpers
from msi.imgmani import remove_masked_elements, select_n_reflectances


class TestImgMani(unittest.TestCase):

    def setUp(self):
        self.msi = helpers.getFakeMsi()
        # set one pixel to special values
        self.specialValue = np.arange(self.msi.get_image().shape[-1]) * 2
        self.msi.get_image()[2, 2, :] = self.specialValue
        # create a segmentation which sets all elements to invalid but the
        # one pixel with the special value
        self.segmentation = np.zeros(self.msi.get_image().shape[0:-1])
        self.segmentation[2, 2] = 1
        # apply this segmentation to the msi
        msimani.apply_segmentation(self.msi, self.segmentation)
        self.image = self.msi.get_image()

    def tearDown(self):
        self.msi = None
        self.specialValue = None
        self.segmentation = None
        self.image = None

    def test_collapse_image(self):
        image = self.image
        newShapedImage = imgmani.collapse_image(image)

        self.assertEqual(newShapedImage.shape,
                        (image.shape[0] * image.shape[1], image.shape[2]),
                        "collapsed image has correct shape")
        np.testing.assert_equal(newShapedImage[2 * 5 + 2, :],
                                       self.msi.get_image()[2, 2, :],
                        "values have been correctly transformed")

    def test_collapse_image_retains_data(self):
        newShapedImage = imgmani.collapse_image(self.image)
        self.msi.get_image()[2, 2, 0] = 5000.

        self.assertEqual(newShapedImage[2 * 5 + 2, 0], 5000.,
                        "collapse_image does not copy data")

    def test_remove_masked_elements(self):
        value = self.msi.get_image()[2, 2, :]
        image_without_masked = remove_masked_elements(self.image)
        np.testing.assert_array_equal(image_without_masked[0, :], value,
                                      "mask correctly removed")
        self.assertEqual(image_without_masked.shape,
                         (1, self.image.shape[-1]),
                         "shape of image without mask correct")

    def test_select_n_reflectances_selects(self):
        n = 10
        new_image = select_n_reflectances(self.image, n)
        self.assertEqual(new_image.shape, (n, self.image.shape[-1]),
                        "correct shape after selection")

    def test_select_n_reflectances_permutes(self):
        image_shape = self.image.shape
        new_first_layer = np.random.random_sample(image_shape[0:-1])
        self.image[:, :, 0] = new_first_layer
        shuffled_image = select_n_reflectances(self.image,
                                        image_shape[0] * image_shape[1])
        # restore_shape
        shuffled_image = np.reshape(shuffled_image, image_shape)
        self.assertFalse(np.allclose(shuffled_image[:, :, 0],
                                     new_first_layer),
                         "image has been shuffled")

    def test_get_bands_from_int(self):
        new_image_bands = imgmani.get_bands(self.image, 2)
        self.assertEqual(new_image_bands.shape, (5, 5, 1),
                         "new image has correct shape")
        self.assertEqual(new_image_bands[2, 2, :], self.specialValue[2],
                         "new image has correct values")

    def test_get_bands_from_array(self):
        new_image_bands = imgmani.get_bands(self.image, np.array([0, 1, 2]))
        self.assertEqual(new_image_bands.shape, (5, 5, 3),
                         "new image has correct shape")
        np.testing.assert_allclose(new_image_bands[2, 2, :],
                                   self.specialValue[:3],
                                   err_msg="new image has correct values")


