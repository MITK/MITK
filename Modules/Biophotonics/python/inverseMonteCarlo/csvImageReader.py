# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 17:48:05 2015

@author: wirkert
"""

import csv
import numpy as np


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

    resultShape = (int(shapeLine[1]), int(shapeLine[2]))

    print 'determined shape to be: ', resultShape

    reflectances = np.loadtxt(csvFilename, skiprows=2, delimiter=";")

    return (resultShape, reflectances)


def csvImageReadAndCorrect(csvFilename):
    """
    read an image and correct it by its dark and flatfield image.
    dark and flatfield image are expected to have the same name
    as the original image with suffixes
    _dark
    _flatfield
    """

    shape, reflectances = csvImageReader(csvFilename)
    dummy, dark         = csvImageReader(csvFilename + "_dark")
    dummy, flatfield    = csvImageReader(csvFilename + "_flatfield")

    correctedReflectances = (reflectances - dark) / flatfield

    return (shape, correctedReflectances)


if __name__ == "__main__":

    shape, reflectances = csvImageReader("data/output/sample_0")
    shape, correctedReflectances = csvImageReadAndCorrect("data/output/sample_0")