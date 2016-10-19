'''
Created on Sep 28, 2015

@author: wirkert
'''


import os
import itertools

# PIL always rescales the image, thus PIL and skimage (which uses PIL) cannot
# be used
import png
import numpy as np

from msi.io.reader import Reader
from msi.msi import Msi


class PngReader(Reader):
    """
    Assumes bitdepth 16bit.
    TODO SW: document and test"""

    def __init__(self):
        pass


    def read(self, fileToRead):
        """ read the msi from pngs.
        The fileToRead is a string prefix, all files starting with this
        prefix will be summarized to one msi"""

        path, file_prefix = os.path.split(fileToRead)
        files = os.listdir(path)
        files_to_read = [os.path.join(path, f) for f in files
                         if f.startswith(file_prefix)]
        files_to_read.sort()
        image_array = [toImage(f)
                       for f in files_to_read]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)
        return msi


def toImage(f):
    reader = png.Reader(f)
    column_count, row_count, pngdata, params = reader.asDirect()
    plane_count = params['planes']
    image_2d = np.vstack(itertools.imap(np.uint16, pngdata))
    # this is needed for rgb images. probably better would be a mean in case
    # we convert "real" rgb data. This is just for rgb images which
    # contain the same values for r,g and b for every pixel.
    image_3d = np.reshape(image_2d,
                         (row_count, column_count, plane_count))
    return image_3d[:, :, 0]

