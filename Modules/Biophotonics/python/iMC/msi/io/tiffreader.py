'''
Created on Sep 28, 2015

@author: wirkert
'''


import os

import Image
import numpy as np

from msi.io.reader import Reader
from msi.msi import Msi


class TiffReader(Reader):
    """
    Assumes bitdepth 16bit on a 12bit camera.
    TODO SW: document and test"""

    def __init__(self):
        pass


    def read(self, fileToRead):
        """ read the msi from tiffs.
        The fileToRead is a string prefix, all files starting with this
        prefix will be summarized to one msi"""

        path, file_prefix = os.path.split(fileToRead)
        files = os.listdir(path)
        files_to_read = [os.path.join(path, f) for f in files
                         if file_prefix[2:] in f]
        files_to_read.sort()
        image_array = [toImage(f)
                       for f in files_to_read]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)
        return msi


def toImage(f):
    im = Image.open(f)
    im_array = np.array(im)
    im_array = im_array >> 4
    return im_array

