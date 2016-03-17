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

    def read(self, file_to_read):
        # our spectrometer like to follow german standards in files, we need
        # to switch to english ones
        transformed=""
        replacements = {',': '.', '\r\n': ''}
        with open(file_to_read) as infile:
            for line in infile:
                for src, target in replacements.iteritems():
                    line = line.replace(src, target)
                transformed = "\n".join([transformed, line])

        for num, line in enumerate(transformed.splitlines(), 1):
            if ">>>>>Begin" in line:
                break

        for num_end, line in enumerate(transformed.splitlines(), 1):
            if ">>>>>End" in line:
                num_end -= 1
                break
        string_only_spectrum = "\n".join(transformed.splitlines()[num:num_end])
        data_vector = np.fromstring(string_only_spectrum,
                                    sep="\t").reshape(-1, 2)
        msi = Msi(data_vector[:, 1],
                  {'wavelengths': data_vector[:, 0] * 10 ** -9})
        return msi

