'''
Created on Oct 26, 2015

@author: wirkert
'''

import numpy as np
import pandas as pd
from sklearn.preprocessing import Normalizer


def preprocess2(df, nr_samples=None, snr=None, movement_noise_sigma=None,
                magnification=None, bands_to_sortout=None):

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
    if bands_to_sortout is not None and bands_to_sortout.size > 0:
        X.drop(X.columns[bands_to_sortout], axis=1, inplace=True)
        snr = np.delete(snr, bands_to_sortout)
    X = X.values
    y = df.layer0[["sao2", "vhb"]]

    # do data magnification
    if magnification is not None:
        X_temp = X
        y_temp = y
        for i in range(magnification - 1):
            X = np.vstack((X, X_temp))
            y = pd.concat([y, y_temp])

    # add noise to reflectances
    camera_noise = 0.
    if snr is not None:
        sigmas = X / snr
        noises = np.random.normal(loc=0., scale=1, size=X.shape)
        camera_noise = sigmas*noises

    movement_noise = 0.
    if movement_noise_sigma is not None:
        nr_bands = X.shape[1]
        nr_samples = X.shape[0]
        # we assume no correlation between neighboring bands
        CORRELATION_COEFFICIENT = 0.0
        movement_variance = movement_noise_sigma ** 2
        movement_variances = np.ones(nr_bands) * movement_variance
        movement_covariances = np.ones(nr_bands-1) * CORRELATION_COEFFICIENT * \
            movement_variance
        movement_covariance_matrix = np.diag(movement_variances) + \
            np.diag(movement_covariances, -1) + \
            np.diag(movement_covariances, 1)
        # percentual sample errors
        sample_errors_p = np.random.multivariate_normal(mean=np.zeros(nr_bands),
                cov=movement_covariance_matrix,
                size=nr_samples)
        # errors w.r.t. the curve height.
        movement_noise = X * sample_errors_p

    X += camera_noise + movement_noise

    X = np.clip(X, 0.00001, 1.)
    # do normalizations
    X = normalize(X)
    return X, y


def preprocess(batch, nr_samples=None, snr=None, movement_noise_sigma=None,
               magnification=None, bands_to_sortout=None):
    X, y = preprocess2(batch, nr_samples, snr, movement_noise_sigma,
                       magnification, bands_to_sortout)

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
