# -*- coding: utf-8 -*-
"""
Created on Mon Aug 10 16:29:20 2015

@author: wirkert
"""

import logging
import numpy as np

import SimpleITK as sitk

from msi.io.reader import Reader
from msi.msi import Msi


class NrrdReader(Reader):

    def __init__(self):
        pass

    def read(self, fileToRead):
        """ read the nrrd image.
        TODO: properties are not correctly extracted from nrrd."""

        image = None

        try:
            reader = sitk.ImageFileReader()
            reader.SetFileName(fileToRead)

            image = reader.Execute()
            image = sitk.GetArrayFromImage(image)

        except RuntimeError as re:
            # image could not be read
            logging.warning("image " + fileToRead +
            " could not be loaded: " +
            str(re))
            # rethrow exception after logging
            raise

        # if image is too low dimensional singleton dimensions
        # are added when saving. Done because sitk can only handle dimensions
        # 2,3,4. This removes these singleton dimensions again.
        squeezed_image = np.squeeze(image)
        msi = Msi(squeezed_image)
        return msi
