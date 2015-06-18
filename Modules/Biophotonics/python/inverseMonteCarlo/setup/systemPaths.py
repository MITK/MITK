# -*- coding: utf-8 -*-
"""
Created on Wed Feb 18 09:49:56 2015

@author: wirkert
"""

import os

def getModuleRootFolder():

    mycwd = os.getcwd()
    os.chdir(os.path.dirname(__file__))
    os.chdir('..')

    moduleRootFolder = os.getcwd() + "/"

    os.chdir(mycwd)

    return moduleRootFolder



def getOutputFolder() :
    """
    standard folder to put output of the inverse monte carlo script
    """

    outputFolder = getModuleRootFolder() + 'data/output/'

    return outputFolder


def initPaths():
    """
    modify these paths so they concide with the ones on your system.
    """

    # the input file without the run specific parameters for ua, us and d:
    infileString = getModuleRootFolder() + 'data/colonTemplate.mci'
    # the output folder for the mc simulations
    # attention: this is relative to your gpumcml path!
    outfolderMC = 'outputMC/'
    # the output folder for the reflectance spectra
    outfolderRS = getOutputFolder()
    gpumcmlDirectory = '/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/'
    gpumcmlExecutable = 'gpumcml.sm_20'

    return infileString, outfolderMC, outfolderRS, gpumcmlDirectory, gpumcmlExecutable

if __name__ == "__main__":

    i, o, o2, g, g2 = initPaths()
