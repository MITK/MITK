'''
Created on Nov 2, 2015

@author: wirkert
'''

import os
import itertools

# PIL always rescales the image, thus PIL and skimage (which uses PIL) cannot
# be used
import Image
import numpy as np

from msi.io.reader import Reader
from msi.msi import Msi


class TiffRingReader(Reader):
    '''
    TODO SW: document and test
    '''

    def __init__(self):
        pass

    def read(self, fileToRead, n):
        """ read the msi from pngs.
        The fileToRead is the first file to read,
        then n files will be read to one msi from a
        sorted file list
        """

        path, file_name = os.path.split(fileToRead)
        files = os.listdir(path)
        files_in_folder = [os.path.join(path, f) for f in files if
                           os.path.isfile(os.path.join(path, f))]

        files_in_folder.sort()
        position = files_in_folder.index(fileToRead)
        # take n images from found position
        image_array = [toImage(f)
                       for f in files_in_folder[position:position + n]]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)
        return msi


def toImage(f):
    im = Image.open(f)
    im_array = np.array(im)
    im_array = im_array >> 4
    return im_array
