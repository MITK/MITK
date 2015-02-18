# -*- coding: utf-8 -*-
"""
Created on Wed Feb 18 09:49:56 2015

@author: wirkert
"""

def initPaths():
    """
    modify these paths so they concide with the ones on your system.
    """
    # the input file without the run specific parameters for ua, us and d:
    infileString = 'data/colonTemplate.mci'
    # the output folder for the mc simulations
    # attention: this is relative to your gpumcml path!
    outfolderMC ='outputMC/'
    # the output folder for the reflectance spectra
    outfolderRS = 'data/output/'
    gpumcmlDirectory = '/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/'
    gpumcmlExecutable = 'gpumcml.sm_20'

    return infileString, outfolderMC, outfolderRS, gpumcmlDirectory, gpumcmlExecutable