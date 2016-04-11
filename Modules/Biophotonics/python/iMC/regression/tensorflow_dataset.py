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


def read_data_set(dataframe_filename, fake_data=False):

    if fake_data:
        data_set = DataSet([], [], fake_data=True)
        return data_set

    df_data_set = pd.read_csv(os.path.join(dir, dataframe_filename),
                              header=[0, 1])

    data_set_images, data_set_labels = preprocess(df_data_set, snr=10.)
    data_set_labels = data_set_labels.values
    data_set = DataSet(data_set_images, data_set_labels)
    return data_set
