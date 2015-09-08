# -*- coding: utf-8 -*-
"""
Created on Thu Aug 13 11:13:31 2015

@author: wirkert
"""


import matplotlib.pyplot as plt
import imgmani
import numpy as np

def plot(msi, axes=None):
    """
    create a plot for the Msi with x axes being the wavelengths and
    y-axes being the corresponding image values (e.g. reflectances, absorptions)
    Takes image masks into account:
    doesn't plot a spectrum containing masked elements
    """
    if axes is None:
        axes = plt.gca()

    sortedIndices = sorted(range(len(msi.get_wavelengths())), key=lambda k: msi.get_wavelengths()[k])
    sortedWavelenghts = msi.get_wavelengths()[sortedIndices]
    # reshape to collapse all but last dimension (which contains reflectances)
    collapsedImage = imgmani.collapse_image(msi.get_image())
    # todo: simply use np.ma.compress_rows

    # print "filtered ", filteredImage.shape
    i = 0
    for i in range(collapsedImage.shape[0]):
        if (collapsedImage[i, 0] is not np.ma.masked):
            axes.plot(sortedWavelenghts, collapsedImage[i, :][sortedIndices], "-o")



