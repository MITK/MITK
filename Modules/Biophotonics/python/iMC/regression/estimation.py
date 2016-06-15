

"""

The MultiSpectral Imaging Toolkit (MSITK)

Copyright (c) German Cancer Research Center,
Computer Assisted Interventions.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE for details

"""
'''
Created on Oct 21, 2015

@author: wirkert
'''

import math
import logging
import time

import tensorflow as tf
import numpy as np
import SimpleITK as sitk

from regression.tensorflow_estimator import multilayer_perceptron, cnn
import msi.imgmani as imgmani


def SAMDistance(x, y):
    return math.acos(np.dot(x, y) / (np.linalg.norm(x) * np.linalg.norm(y)))


def estimate_image(msi, regressor):
    """given an Msi and an regressor estimate the parmaeters for this image

    Paramters:
        msi: multi spectral image
        regressor: regressor, must implement the predict method"""

    # estimate parameters
    collapsed_msi = imgmani.collapse_image(msi.get_image())
    # in case of nan values: set to 0
    collapsed_msi[np.isnan(collapsed_msi)] = 0.
    collapsed_msi[np.isinf(collapsed_msi)] = 0.

    start = time.time()
    estimated_parameters = regressor.predict(collapsed_msi)
    end = time.time()
    estimation_time = end - start
    logging.info("time necessary for estimating image parameters: " +
        str(estimation_time) + "s")
    # restore shape
    feature_dimension = 1
    if len(estimated_parameters.shape) > 1:
        feature_dimension = estimated_parameters.shape[-1]

    estimated_paramters_as_image = np.reshape(
            estimated_parameters, (msi.get_image().shape[0],
                                   msi.get_image().shape[1],
                                   feature_dimension))
    # save as sitk nrrd.
    sitk_img = sitk.GetImageFromArray(estimated_paramters_as_image,
                                 isVector=True)
    return sitk_img, estimation_time


def estimate_image_tensorflow(msi, model_checkpoint_dir):
    # estimate parameters
    collapsed_msi = imgmani.collapse_image(msi.get_image())
    # in case of nan values: set to 0
    collapsed_msi[np.isnan(collapsed_msi)] = 0.
    collapsed_msi[np.isinf(collapsed_msi)] = 0.


    tf.reset_default_graph()

    keep_prob = tf.placeholder("float")
    nr_wavelengths = len(msi.get_wavelengths())
    x = tf.placeholder("float", [None, nr_wavelengths, 1, 1])

    x_test_image = np.reshape(msi.get_image(), [-1, nr_wavelengths, 1, 1])

    # Construct the desired model
    # pred, regularizers = multilayer_perceptron(x, nr_wavelengths, 100, 1,
    #                                            keep_prob)
    pred = cnn(x, 1, keep_prob)

    # Initializing the variables
    init = tf.initialize_all_variables()

    saver = tf.train.Saver()

    with tf.Session() as sess:
        sess.run(tf.initialize_all_variables())
        # restore model:
        ckpt = tf.train.get_checkpoint_state(model_checkpoint_dir)

        if ckpt and ckpt.model_checkpoint_path:
            saver.restore(sess, ckpt.model_checkpoint_path)

            start = time.time()
            estimated_parameters = pred.eval({x: x_test_image,
                                              keep_prob:1.0})
            end = time.time()
            estimation_time = end - start
            logging.info("time necessary for estimating image parameters: " +
                str(estimation_time) + "s")
    # restore shape
    feature_dimension = 1
    if len(estimated_parameters.shape) > 1:
        feature_dimension = estimated_parameters.shape[-1]

    estimated_paramters_as_image = np.reshape(
            estimated_parameters, (msi.get_image().shape[0],
                                   msi.get_image().shape[1],
                                   feature_dimension))
    # save as sitk nrrd.
    sitk_img = sitk.GetImageFromArray(estimated_paramters_as_image,
                                 isVector=True)

    return sitk_img, estimation_time


def standard_score(estimator, X, y):
    """our standard scoring method is the median absolute error"""
    return np.median(np.abs(estimator.predict(X) - y))

