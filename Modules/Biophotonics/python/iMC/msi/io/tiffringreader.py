'''
Created on Nov 2, 2015

@author: wirkert
'''

import os
import logging

# PIL always rescales the image, thus PIL and skimage (which uses PIL) cannot
# be used
import Image
import numpy as np

from msi.io.reader import Reader
from msi.msi import Msi
import msi.msimanipulations as msimani


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
                           os.path.isfile(os.path.join(path, f)) and
                           f.endswith('.tiff')]

        files_in_folder.sort()
        position = files_in_folder.index(fileToRead)
        # take n images from found position
        image_array = [toImage(f)
                       for f in files_in_folder[position:position + n]]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)

        segmentation = None
        try:
            segmentation_array = [toSegmentation(f) \
                                  for f in files_in_folder[position:position + n]]
            segmentation = reduce(lambda x, y: x & y, segmentation_array)

        except:
            logging.info("didn't find segmentation for all images")
        return msi, segmentation

def toImage(f):
    im = Image.open(f)
    im_array = np.array(im)
    im_array = im_array >> 4
    return im_array

def toSegmentation(f):
    seg = np.load(f + ".seg.npy")
    return seg
