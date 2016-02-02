'''
Created on Oct 19, 2015

@author: wirkert
'''

import pickle
import os

import skimage.restoration as res
import numpy as np
import matplotlib.pylab as plt
import SimpleITK as sitk

from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
import msi.plot as msiplot
import msi.imgmani as imgmani
from scipy.ndimage.filters import gaussian_filter

def save_as_np(filename, prefix):
    f = open(filename)
    batch = pickle.load(f)
    np.save("/media/wirkert/data/Data/temp/" + prefix + "_reflectances",
            batch.reflectances)
    np.save("/media/wirkert/data/Data/temp/" + prefix + "_saO2",
            batch.layers[0][:, 1])

if __name__ == '__main__':

    image_file_folder = "/media/wirkert/data/Data/2015_11_12_IPCAI_in_silico/mc_data2"

    onlyfiles = [ os.path.join(image_file_folder, f) for f in os.listdir(image_file_folder) if
                 os.path.isfile(os.path.join(image_file_folder, f)) ]
    onlyfiles.sort()

    for f in onlyfiles:
        file = open(f)
        b = pickle.load(file)
        print f
        print np.sum(b.reflectances <= 0)
        print np.sum(np.isnan(b.reflectances))
