# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 17:48:05 2015

Reads the csv images provided by our MITK miniapp.
The image is supposed to be structured as follows:

1. line: arbitrary data
2. line: image shape;1024;768
3. line - 1024x768 + 2 line: multispectral spectra.

e.g.: 4000;8144;3776;4880;9408;6048;3808;4704
for an image with 8 spectral bands. This is one pixel.

image shape;1024;768 is of course only an example for an image resolution of 1024x768

@author: wirkert
"""

import csv
import numpy as np
import Image


def csvImageReader(csvFilename):
    """
    read in image from csv format.
    attention: enter name without suffix (.csv will be added automatically)
    """
    csvFilename = csvFilename + ".csv"

    print "loading image", csvFilename


    with open(csvFilename, 'r') as myFile:
        reader=csv.reader(myFile, delimiter=";")
        for i, row in enumerate(reader):
            if i == 1:
                shapeLine = row
                break

    # for some reason, shapes are flipped from itk to csv
    resultShape = (int(shapeLine[2]), int(shapeLine[1]))

    print 'determined shape to be: ', resultShape

    reflectances = np.loadtxt(csvFilename, skiprows=2, delimiter=";")

    return (resultShape, reflectances)


def csvMultiSpectralImageReader2(csvFilename):
    shape, reflectances = csvImageReader(csvFilename)
    dummy, flatfieldsegmentation    = csvImageReader(csvFilename + "_flatfieldsegmentation")
    dummy, trainsegmentation = csvImageReader(csvFilename + "_trainsegmentation")
    dummy, testsegmentation =csvImageReader(csvFilename + "_testsegmentation")

    flatfield = reflectances[np.nonzero(flatfieldsegmentation)[0], :]
    flatfield = np.mean(flatfield, axis = 0)

    correctedReflectances = reflectances / flatfield

    return (shape, correctedReflectances, trainsegmentation, testsegmentation)


def csvMultiSpectralImageReader(csvFilename):
    """
    read an image and correct it by its dark and flatfield image.
    dark and flatfield image are expected to have the same name
    as the original image with suffixes
    _dark
    _flatfield
    Also returns the segmentation of the multispectral image, expecting
    the suffixes:
    _trainsegmentation
    _testsegmentation
    Segmentation will have value > 0 if pixel is segmented, 0 otherwise.
    """

    shape, reflectances = csvImageReader(csvFilename)
    dummy, dark         = csvImageReader(csvFilename + "_dark")
    dummy, flatfield    = csvImageReader(csvFilename + "_flatfield")
    dummy, trainsegmentation = csvImageReader(csvFilename + "_trainsegmentation")
    dummy, testsegmentation =csvImageReader(csvFilename + "_testsegmentation")

    correctedReflectances = (reflectances - dark) / (flatfield - dark)

    return (shape, correctedReflectances, trainsegmentation, testsegmentation)


if __name__ == "__main__":

    #%% load images
    shape, reflectances = csvImageReader("data/output/sample_0")
    shape, correctedReflectances, trainseg, testseg = csvMultiSpectralImageReader("data/output/sample_0")

    #%% save again

    # save one wavelength of the uncorrected image to see if reading worked
    uncorrectedImage_0 = np.reshape(reflectances[:,0], shape)

    #Rescale to 0-255 and convert to uint8
    rescaled = (255.0 / uncorrectedImage_0.max() * (uncorrectedImage_0 - uncorrectedImage_0.min())).astype(np.uint8)

    im = Image.fromarray(rescaled)
    im.save("data/output/" + "testUncorrectedImage_0.tiff")

    # save the corrected images to see if reading and correction worked.
    for i in np.arange(0,correctedReflectances.shape[1]):
        correctedImage_i = np.reshape(correctedReflectances[:,i], shape)
        im = Image.fromarray(correctedImage_i)
        im.save("data/output/" + "testCorrectedImage_" + str(i) + ".tiff")
