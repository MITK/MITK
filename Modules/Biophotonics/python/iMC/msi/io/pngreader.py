'''
Created on Sep 28, 2015

@author: wirkert
'''


import numpy as np
import os

import skimage.io

from msi.io.reader import Reader
from msi.msi import Msi



class PngReader(Reader):
    """TODO SW: document and test"""

    def __init__(self):
        pass


    def read(self, fileToRead):
        """ read the msi from pngs.
        The fileToRead is a string prefix, all files starting with this
        prefix will be summarized to one msi"""

        path, file_prefix = os.path.split(fileToRead)
        files = os.listdir(path)
        files_to_read = [f for f in files if f.startswith(file_prefix)]
        files_to_read.sort()
        image_array = [skimage.io.imread(os.path.join(path, f))
                       for f in files_to_read]
        image = reduce(lambda x, y: np.dstack((x, y)), image_array)

        msi = Msi(image)
        return msi
