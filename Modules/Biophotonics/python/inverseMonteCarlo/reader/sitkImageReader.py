# -*- coding: utf-8 -*-
"""
Created on Fri Apr 17 16:21:07 2015

@author: wirkert
"""


import SimpleITK as sitk

def read (fileName):
    """
    load a simpleitk image and put its data array in a numpy array

    Params:
    ____
    fileName
        the filename _without_ extension. .nrrd will be expected.

    Returns:
    ____
    the data of the image as numpy array. all other information in the SimpleITK
    image is ignored
    """
    image = None

    try:
        reader = sitk.ImageFileReader()
        reader.SetFileName(fileName + ".nrrd")

        image = reader.Execute()
        image = sitk.GetArrayFromImage(image)

    except RuntimeError:
        print "image " + fileName + ".nrrd not found and thus not loaded."

    return image

def readMS (fileName):
    """
    reads the information necessary for multispectral image processing.

    Returns:
    ____
    (image, trainSeg, testSeg)
        the image is the multispectral (ms) image corrected by an
    (optional) white balance image. If available, train- and test segmentations are
    retured. They are of the same 2D size as the ms image and have as values either 1 (inclueded in segmentation)
    or 0 (not included in segmentation)
    """

    rawMsImage        = read(fileName)
    whitebalanceImage = read(fileName + "_flatfield")
    trainSegmentation = read(fileName + "_trainSeg")
    testSegmentation  = read(fileName + "_testSeg")

    msImage = rawMsImage / whitebalanceImage

    return msImage, trainSegmentation, testSegmentation