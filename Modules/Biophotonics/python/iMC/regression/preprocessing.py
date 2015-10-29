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
    if w_percent is None:
        w_percent = 0.
    if magnification is not None:
        r_new = batch.reflectances
        layers_new = working_batch.layers
        for i in range(magnification - 1):
            r_new = np.vstack((r_new, batch.reflectances))
            for i in range(len(layers_new)):
                layers_new[i] = np.vstack((layers_new[i], batch.layers[i]))
        working_batch.reflectances = r_new
        working_batch.layers = layers_new

    # extract nr_samples samples from data
    if nr_samples is not None:
        mcmani.select_n(working_batch, nr_samples)
    # get reflectance and oxygenation
    reflectances = working_batch.reflectances
    y = working_batch.layers[0][:, 1]
    # add noise to reflectances
    if not np.isclose(w_percent, 0.):
        noises = np.random.normal(loc=0., scale=w_percent,
                                  size=reflectances.shape)
        reflectances += noises * reflectances
    reflectances = np.clip(reflectances, 0.00001, 1.)
    X = reflectances
    return X, y

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
