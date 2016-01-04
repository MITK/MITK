'''
Created on Oct 26, 2015

@author: wirkert
'''

import numpy as np
from sklearn.preprocessing import Normalizer


def preprocess2(df, nr_samples=None, w_percent=None, magnification=None,
               bands_to_sortout=None):

    df.drop(bands_to_sortout, axis=1, level=1, inplace=True)

    # first set 0 reflectances to nan
    df["reflectances"] = df["reflectances"].replace(to_replace=0.,
                                                    value=np.nan)
    # remove nan
    df.dropna(inplace=True)

    # extract nr_samples samples from data
    if nr_samples is not None:
        df = df.sample(nr_samples)

    # get reflectance and oxygenation
    X = df.reflectances
    y = df.layer0[["sao2", "vhb"]]

    # do data magnification
    if magnification is not None:
        X_temp = X
        y_temp = y
        for i in range(magnification - 1):
            X = np.vstack((X, X_temp))
            y = np.vstack((y, y_temp))
    # add noise to reflectances
    if w_percent is not None and (w_percent > 0.):
        noises = np.random.normal(loc=0., scale=w_percent,
                                  size=X.shape)
        X += noises * X
    X = np.clip(X, 0.00001, 1.)
    # do normalizations
    X = normalize(X)
    return X, y



def preprocess(batch, nr_samples=None, w_percent=None, magnification=None,
               bands_to_sortout=None):
    X, y = preprocess2(batch, nr_samples, w_percent, magnification,
                       bands_to_sortout)

    return X, y["sao2"]


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
