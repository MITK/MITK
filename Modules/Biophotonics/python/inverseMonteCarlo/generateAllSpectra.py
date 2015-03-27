# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 18:13:13 2015

this script will call the other generate scripts to generate all the data neccesary
for our experiments

@author: wirkert
"""


from generate.noisyRandom      import noisyRandom
from generate.randomUniform    import randomUniform
from generate.perfectGrid      import perfectGrid

# training noisy spectra
noisyRandom("NoisyRandomTraining")

# testing noisy spectra
noisyRandom("NoisyRandomTesting")

# random uniform non-noisy spectra
randomUniform("UniformRandom")

# training perfect data on grid
perfectGrid("PerfectGridTraining")

