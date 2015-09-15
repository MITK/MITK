# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 11:13:31 2015

@author: wirkert
"""


import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable

import imgmani

def plot(msi, axes=None):
    """
    create a plot for the Msi with x axes being the wavelengths and
    y-axes being the corresponding image values (e.g. reflectances, absorptions)
    Takes image masks into account:
    doesn't plot a spectrum containing masked elements
    """
    if axes is None:
        axes = plt.gca()

    sortedIndices = sorted(range(len(msi.get_wavelengths())),
                           key=lambda k: msi.get_wavelengths()[k])
    sortedWavelenghts = msi.get_wavelengths()[sortedIndices]
    # reshape to collapse all but last dimension (which contains reflectances)
    collapsedImage = imgmani.collapse_image(msi.get_image())
    # todo: simply use np.ma.compress_rows

    # print "filtered ", filteredImage.shape
    i = 0
    for i in range(collapsedImage.shape[0]):
        if (collapsedImage[i, 0] is not np.ma.masked):
            axes.plot(sortedWavelenghts,
                      collapsedImage[i, :][sortedIndices], "-o")


def plot_images(msi):
    """plot the images as a 2d image array, one image for
    each wavelength."""
    nr_wavelengths = msi.get_image().shape[-1]
    f, axarr = plt.subplots(1, nr_wavelengths)
    for i, a in enumerate(axarr):
        one_band_image = imgmani.get_bands(msi.get_image(), i)
        im = a.imshow(np.squeeze(one_band_image))
        a.set_title("band nr " + str(i), fontsize=5)
        divider_dsp = make_axes_locatable(a)
        cax_dsp = divider_dsp.append_axes("right", size="10%", pad=0.1)
        cbar = plt.colorbar(im, cax=cax_dsp)
        cbar.ax.tick_params(labelsize=5)
        a.xaxis.set_visible(False)
        a.yaxis.set_visible(False)



# TODO: write this function
# def plotMeanError(msi, axes=None):
#     """
#     create a plot for the Msi with x axes being the wavelengths and
#     y-axes being the corresponding mean image values
#     (e.g. reflectances, absorptions). Plotts also variantion bands
#     Takes image masks into account:
#     doesn't plot a spectrum containing masked elements
#     """
#     if axes is None:
#         axes = plt.gca()
#
#     sortedIndices = sorted(range(len(msi.get_wavelengths())),
#                            key=lambda k: msi.get_wavelengths()[k])
#     sortedWavelenghts = msi.get_wavelengths()[sortedIndices]
#     # reshape to collapse all but last dimension (which contains reflectances)
#     collapsedImage = imgmani.collapse_image(msi.get_image())
#     # todo: simply use np.ma.compress_rows
#
#     # print "filtered ", filteredImage.shape
#     i = 0
#     for i in range(collapsedImage.shape[0]):
#         if (collapsedImage[i, 0] is not np.ma.masked):
#             axes.plot(sortedWavelenghts,
#                       collapsedImage[i, :][sortedIndices], "-o")
