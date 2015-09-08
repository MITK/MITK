# -*- coding: utf-8 -*-
"""
Created on Fri Aug  7 12:04:18 2015

@author: wirkert
"""

import numpy as np
from msi.io.reader import Reader
from msi.msi import Msi

class SpectrometerReader(Reader):

    def __init__(self):
        pass


    def read(self, fileToRead):

        with open(fileToRead) as myFile:
            for num, line in enumerate(myFile, 1):
                if ">>>>>Begin Spectral Data<<<<<" in line:
                    break

        dataVector = np.loadtxt(fileToRead, skiprows=num)


        msi = Msi(dataVector[:, 1], {'wavelengths' : dataVector[:, 0] * 10 ** -9})

        return msi

