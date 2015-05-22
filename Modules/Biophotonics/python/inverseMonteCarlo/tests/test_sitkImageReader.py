# -*- coding: utf-8 -*-
"""
Created on Fri Apr 17 16:22:34 2015

@author: wirkert
"""

import unittest
import reader.sitkImageReader as reader

class test_sitkImageReader(unittest.TestCase):

    def setUp(self):
        self.image            = reader.read('data/allBands')
        self.notExistingImage = reader.read('data/asdf')
        # note: this test has to be changed to an image lying in mitk testdata

    def test_readDoesNotCrash(self):
        # if we got this far, at least an image was read.
        self.assertTrue(len(self.image.shape) == 3)

    def test_readNonExistingImageReturnsNone(self):
        self.assertTrue(self.notExistingImage is None)