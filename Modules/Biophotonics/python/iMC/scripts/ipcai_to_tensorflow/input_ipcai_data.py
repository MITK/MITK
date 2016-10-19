"""Functions for downloading and reading ipcai data."""
from __future__ import print_function

import os

import numpy
import pandas as pd

from regression.preprocessing import preprocess


class DataSet(object):
    def __init__(self, images, labels, fake_data=False):
        if fake_data:
            self._num_examples = 10000
        else:
            assert images.shape[0] == labels.shape[0], (
                "images.shape: %s labels.shape: %s" % (images.shape,
                                                       labels.shape))
            self._num_examples = images.shape[0]
            images = images.astype(numpy.float32)
        self._images = images
        self._labels = labels
        if self._labels.ndim == 1:
            self._labels = self._labels[:, numpy.newaxis]
        self._epochs_completed = 0
        self._index_in_epoch = 0

    @property
    def images(self):
        return self._images

    @property
    def labels(self):
        return self._labels

    @property
    def num_examples(self):
        return self._num_examples

    @property
    def epochs_completed(self):
        return self._epochs_completed

    def next_batch(self, batch_size, fake_data=False):
        """Return the next `batch_size` examples from this data set."""
        if fake_data:
            fake_image = [1.0 for _ in xrange(784)]
            fake_label = 0
            return [fake_image for _ in xrange(batch_size)], [
                fake_label for _ in xrange(batch_size)]
        start = self._index_in_epoch
        self._index_in_epoch += batch_size
        if self._index_in_epoch > self._num_examples:
            # Finished epoch
            self._epochs_completed += 1
            # Shuffle the data
            perm = numpy.arange(self._num_examples)
            numpy.random.shuffle(perm)
            self._images = self._images[perm]
            self._labels = self._labels[perm]

            # Start next epoch
            start = 0
            self._index_in_epoch = batch_size
            assert batch_size <= self._num_examples
        end = self._index_in_epoch
        return self._images[start:end], self._labels[start:end]


def read_data_sets(dir, fake_data=False):

    class DataSets(object):
        pass
    data_sets = DataSets()
    if fake_data:
        data_sets.train = DataSet([], [], fake_data=True)
        data_sets.validation = DataSet([], [], fake_data=True)
        data_sets.test = DataSet([], [], fake_data=True)
        return data_sets

    TRAIN_IMAGES = "ipcai_revision_colon_mean_scattering_train_all_spectrocam.txt"
    TEST_IMAGES = "ipcai_revision_colon_mean_scattering_test_all_spectrocam.txt"

    df_train = pd.read_csv(os.path.join(dir, TRAIN_IMAGES), header=[0, 1])
    df_test = pd.read_csv(os.path.join(dir, TEST_IMAGES), header=[0, 1])

    train_images, train_labels = preprocess(df_train, snr=10.)
    test_images, test_labels = preprocess(df_test, snr=10.)

    train_labels = train_labels.values
    test_labels = test_labels.values

    VALIDATION_SIZE = 1

    validation_images = train_images[:VALIDATION_SIZE]
    validation_labels = train_labels[:VALIDATION_SIZE]
    train_images = train_images[VALIDATION_SIZE:]
    train_labels = train_labels[VALIDATION_SIZE:]
    data_sets.train = DataSet(train_images, train_labels)
    data_sets.validation = DataSet(validation_images, validation_labels)
    data_sets.test = DataSet(test_images, test_labels)
    return data_sets