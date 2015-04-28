# -*- coding: utf-8 -*-
"""
Created on Thu Feb 26 17:53:53 2015

@author: wirkert
"""

import sys

sys.path.append('/home/wirkert/workspace/promotion/MITK/Modules/Biophotonics/python/inverseMonteCarlo')

import os
from setup import data
from setup import systemPaths
import Image
import numpy as np
import csv

from estimateParametersRealImage import estimateParametersRealImage





#%% little helper function
def saveResults(result, shape, r2Score, dataFolder, imageName):
    for i in np.arange(0,result.shape[1]):
        parameterImage_i = np.reshape(result[:,i], shape)
        im = Image.fromarray(parameterImage_i)
        im.save(dataFolder + imageName + "_parameterImage_" + str(i) + ".tiff")


    with open(dataFolder + "result_" + imageName + '.csv', 'wb') as csvfile:
        spamwriter = csv.writer(csvfile, delimiter=';',
                                quotechar='|', quoting=csv.QUOTE_MINIMAL)
        spamwriter.writerow([imageName])
        spamwriter.writerow(['r2Score', str(r2Score)])




# the folder with the reflectance spectra
dataFolder  =  systemPaths.getOutputFolder()



##%% Evaluate CC2 Mitte links data
#imageToLoad = "CC2_links"
#
#
###%% get data
#trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation = \
#    data.realImage(dataFolder, imageToLoad)
#
####%% do evaluation with out domain adaptation
#resName = imageToLoad + "_NODA"
#resultCC2linksNODA, r2ScoreCC2linksNODA, backProjectedReflectancesCC2linksNODA, realReflectanceCC2linksNODA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation,  False)
#saveResults(resultCC2linksNODA, shape, r2ScoreCC2linksNODA, dataFolder, resName)


#%% Evaluate CC2 Mitte rechts data
imageToLoad = "CC2_rechts"


##%% get data
trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation = \
    data.realImage(dataFolder, imageToLoad)

####%% do evaluation with out domain adaptation
#resName = imageToLoad + "_NODA"
#resultCC2rechtsNODA, r2ScoreCC2rechtsNODA, backProjectedReflectancesCC2rechtsNODA, realReflectanceCC2rechtsNODA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation,  False)
#saveResults(resultCC2rechtsNODA, shape, r2ScoreCC2rechtsNODA, dataFolder, resName)

###%% do evaluation with domain adaptation
resName = imageToLoad + "_DA"
resultCC2rechtsDA, r2ScoreCC2rechtsDA, backProjectedReflectancesCC2rechtsDA, realReflectanceCC2rechtsDA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation,  True)
saveResults(resultCC2rechtsDA, shape, r2ScoreCC2rechtsDA, dataFolder, resName)



##%% Evaluate CC1 unten data
#imageToLoad = "CC1"
#
#
###%% get data
#trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation = \
#    data.realImage(dataFolder, imageToLoad)
#
####%% do evaluation with domain adaptation
##resName = imageToLoad + "_DA"
##resultCC1DA, r2ScoreCC1DA, backProjectedReflectancesCC1DA, realReflectancesCC1DA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation,  True)
##saveResults(resultCC1DA, shape, r2ScoreCC1DA, dataFolder, resName)
#
#
##%% do evaluation without domain adaptation
#resName = imageToLoad + "_NODA"
#resultCC1NODA, r2ScoreCC1NODA, backProjectedReflectancesCC1NODA, realReflectancesCC1NODA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation, False)
#saveResults(resultCC1NODA, shape, r2ScoreCC1NODA, dataFolder, resName)
#
#
###%% Evaluate CC1 middle data
#imageToLoad = "CC1_2"
#
#
##%% get data
#trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation = \
#    data.realImage(dataFolder, imageToLoad)
#
###%% do evaluation with domain adaptation
##resultCC2DA, r2ScoreCC2DA, backProjectedReflectancesCC2DA, realReflectancesCC2DA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation, True)
##saveResults(resultCC2DA, shape, r2ScoreCC2DA, dataFolder, imageToLoad + "_DA")
#
##%% do evaluation without domain adaptation
#resultCC2NODA, r2ScoreCC2NODA, backProjectedReflectancesCC2NODA, realReflectancesCC2NODA = estimateParametersRealImage(trainingParameters, trainingReflectances, shape, image, trainsegmentation, testsegmentation, False)
#saveResults(resultCC2NODA, shape, backProjectedReflectancesCC2NODA, dataFolder, imageToLoad + "_NODA")