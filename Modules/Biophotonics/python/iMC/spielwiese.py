'''
Created on Oct 19, 2015

@author: wirkert
'''

import pickle

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

    r = NrrdReader()

    msi = r.read("/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/processed/" +
           "image sample .nrrd")
    seg = r.read("/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/" +
                 "noise_segmentation.nrrd")

    image = msi.get_image()

#     sitk_reader = sitk.ImageFileReader()
#     sitk_reader.SetFileName("/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/processed/" +
#            "image sample .nrrd")
#     sitk_image = sitk_reader.Execute()
#
#
#     gaussian = sitk.SmoothingRecursiveGaussianImageFilter()
#     gaussian.SetSigma(100.)
#     smooth_image = gaussian.Execute(sitk_image)

    filtered_image = gaussian_filter(image.astype(np.float), 2)
    msi.set_image(filtered_image)
    msimani.apply_segmentation(msi, seg)
    msi.set_wavelengths(np.arange(470, 680, 10))
    msiplot.plotMeanError(msi)
#     plt.show()
#
#     image_res = res.denoise_tv_chambolle(image.astype(np.float),
#                                          weight=1000, n_iter_max=100,
#                                          multichannel=True)

    w = NrrdWriter(msi)
    w.write("/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/test.nrrd")

#     img = sitk.GetImageFromArray(smooth_image, isVector=True)
#     sitk.WriteImage(img, "/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/test.nrrd")

#     save_as_np("/media/wirkert/data/Data/" +
#         "2015_11_12_IPCAI_in_silico/processed/" +
#         "generic_tissue_train_all_camera.imc",
#         "hard_train_2")
#
#     save_as_np("/media/wirkert/data/Data/" +
#         "2015_11_12_IPCAI_in_silico/processed/" +
#         "generic_tissue_gaussian_all_camera.imc",
#         "gaussian_train_2")
