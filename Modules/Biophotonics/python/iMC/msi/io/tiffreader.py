'''
Created on Sep 28, 2015

@author: wirkert
'''


import os

import Image
import scipy
import numpy as np

from msi.io.reader import Reader
from msi.msi import Msi


def sort_by_filter(s1, s2):
    '''
    Sorting function which takes two strings and sorts them lexigraphically by
    the last character before the file extension.

    say:

    blabla_w2_F0.tiff < blabla_w0_F1.tiff

    This is done to sort by the filter index, which is always written as the
    last thing by the tiff writer.
    '''
    s1_prefix = os.path.splitext(s1)[0]
    s2_prefix = os.path.splitext(s2)[0]

    result = 0
    if s1_prefix < s2_prefix:
        result = 1
    elif s1_prefix > s2_prefix:
        result = -1

    return result


class TiffReader(Reader):
    """
    AAATTTEEENTION: Some big bug hiding somewhere here, ordering of files
    is corrupted, no time now to fix. TODO SW

    Assumes bitdepth 16bit on a 12bit camera.
    TODO SW: document and test"""

    # static member to globally set resizing for all images read by the reader
    # 1.0 for no resizing
    RESIZE_FACTOR = 1.0

    def __init__(self, shift_bits=4):
        self.shift_bits = shift_bits

    def read(self, file_to_read, resize_factor=None,
             sorting_function=sort_by_filter):
        """ read the msi from tiffs.
        The fileToRead is a string prefix, all files starting with this
        prefix will be summarized to one msi. they will be sorted as specified
        in the sorting_function

        Args:
            sorting_function: the function which defines the sorting of the
                strings that match the prefix. Pass none if normal
                lexicographical sorting is wished
            file_to_read: the prefix of the tiff file which shall be read
        """

        if resize_factor is None:
            resize_factor = TiffReader.RESIZE_FACTOR

        path, file_prefix = os.path.split(file_to_read)
        files = os.listdir(path)
        files_to_read = [os.path.join(path, f) for f in files
                         if file_prefix[2:] in f]
        files_to_read.sort(cmp=sorting_function)
        image_array = [self.to_image(f, resize_factor=resize_factor)
                       for f in files_to_read]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)
        return msi

    def to_image(self, f, resize_factor):
        im = Image.open(f)
        im_array = np.array(im)

        im_array >>= self.shift_bits

        if do_resize(resize_factor):
            im_array = scipy.misc.imresize(im_array, resize_factor,
                                           interp="bilinear", mode="F")

        return im_array.astype('float')


def do_resize(resize_factor):
    return not np.isclose(resize_factor, 1.0) and (resize_factor is not None)
