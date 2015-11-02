'''
Created on Oct 26, 2015

@author: wirkert
'''

import copy

import numpy as np
from sklearn.preprocessing import Normalizer

import mc.mcmanipulations as mcmani

def preprocess(batch, nr_samples=None, w_percent=None, magnification=None,
               bands_to_sortout=None):
    working_batch = copy.deepcopy(batch)
    mcmani.sortout_bands(working_batch, bands_to_sortout)
    # extract nr_samples samples from data
    if nr_samples is not None:
        mcmani.select_n(working_batch, nr_samples)
    # get reflectance and oxygenation
    X = working_batch.reflectances
    y = working_batch.layers[0][:, 1]
    y = y[:, np.newaxis]
    # do data magnification
    if magnification is not None:
        X_temp = X
        y_temp = y
        for i in range(magnification - 1):
            X = np.vstack((X, X_temp))
            y = np.vstack((y, y_temp))
    # add noise to reflectances
    if w_percent is not None:
        noises = np.random.normal(loc=0., scale=w_percent,
                                  size=X.shape)
        X += noises * X
    X = np.clip(X, 0.00001, 1.)
    # do normalizations
    X = normalize(X)
    return X, np.squeeze(y)

def normalize(X):
    # normalize reflectances
    normalizer = Normalizer(norm='l1')
    X = normalizer.transform(X)
    # reflectances to absorption
    absorptions = -np.log(X)
    X = absorptions
    # get rid of sorted out bands
    normalizer = Normalizer(norm='l2')
    X = normalizer.transform(X)
    return X
