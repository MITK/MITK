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


# training non-uniform spectra
randomNonUniform("nonUniformRandomTrainingData")

# testing non-uniform spectra
randomNonUniform("nonUniformRandomTestingData")

# training noisy spectra
#noisyRandom("noisyRandomTrainingData")

# testing noisy spectra
noisyRandom("noisyRandomTestingData")

# training random spectra
#randomUniform("uniformRandomTrainingData")

# testing random spectra
#randomUniform("uniformRandomTestingData")

# training perfect data on grid
#perfectGrid("perfectGridTrainingData")

# testing perfect data on grid
#perfectGrid("perfectGridTestingData")