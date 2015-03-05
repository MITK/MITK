# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 18:13:13 2015

this script will call the other generate scripts to generate all the data neccesary
for our experiments

@author: wirkert
"""


from generate.noisyRandom      import noisyRandom
from generate.randomNonUniform import randomNonUniform
from generate.randomUniform    import randomUniform
from generate.perfectGrid      import perfectGrid

# training noisy spectra
#noisyRandom("noisyTrainingData")

# testing noisy spectra
#noisyRandom("noisyTestingData")

# training non-uniform spectra
#randomNonUniform("nonUniformTrainingData")

# testing non-uniform spectra
#randomNonUniform("nonUniformTestingData")

# testing random spectra
#randomUniform("UniformTestingData")

# normal perfect data on grid
#perfectGrid("PerfectGridTestingData")