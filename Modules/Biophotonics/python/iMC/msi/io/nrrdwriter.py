# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 09:48:18 2015

@author: wirkert
"""

import logging
import numpy as np

import SimpleITK as sitk

from msi.io.writer import Writer


class NrrdWriter(Writer):

    def __init__(self, msiToWrite):
        """
        initialize the write with a specific multi spectral image (class Msi)
        """
        self.msiToWrite = msiToWrite

    def write(self, uri):
        """
        write the msi image to the specified uri
        """
        img_to_write = self.msiToWrite.get_image()

        # sitk can only write images of dimension 2,3,4. This hack is
        # to fake 1d images as being 2d. 1d images e.g. occure after taking
        # the mean of an image.
        if len(img_to_write.shape) == 1:
            img_to_write = np.reshape(img_to_write, (1, 1, img_to_write.shape[0]))

        img = sitk.GetImageFromArray(img_to_write, isVector=True)
        sitk.WriteImage(img, uri)
        logging.info("written file " + uri + " to disk")
        return None
