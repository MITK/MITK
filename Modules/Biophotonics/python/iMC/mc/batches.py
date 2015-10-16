'''
Created on Oct 15, 2015

@author: wirkert
'''

import copy

import numpy as np
from sklearn.preprocessing import MinMaxScaler

class AbstractBatch(object):
    """summarizes a batch of simulated mc spectra"""

    def __init__(self):
        self.reflectances = np.array([])
        self.layers = []  # list of np arrays, one for each layer
        self.nr_photons = np.array([])
        self.wavelengths = np.array([])
        # by default random uniform samples will be generated
        self.generator = np.random.random_sample

    def create_parameters(self, nr_samples):
        """create the parameters for the batch, the simulation has
        to create the resulting reflectances"""
        pass

    def join(self, other_batch):
        """helper method to join two batches"""
        joined_batch = copy.copy(self)

        f = lambda x, y: np.append(x, y, axis=0)

        joined_batch.layers = map(f, joined_batch.layers, other_batch.layers)
        joined_batch.reflectances = f(joined_batch.reflectances,
                                       other_batch.reflectances)

        np.append(joined_batch.reflectances, other_batch.reflectances, axis=0)
        np.append(joined_batch.l1, other_batch.l1, axis=0)
        np.append(joined_batch.l2, other_batch.l2, axis=0)
        np.append(joined_batch.l3, other_batch.l3, axis=0)
        # next two should be the same for two batches from the same experiment
        np.testing.assert_almost_equal(joined_batch.wavelengths,
                                              other_batch.wavelengths)
        np.testing.assert_almost_equal(joined_batch.nr_photons,
                                              other_batch.nr_photons)
        return joined_batch


class GenericBatch(object):
    """generic three layer batch """

    def __init__(self):
        super(AbstractBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""
        samples = self.generator((nr_samples, 5))
        # scale samples to correct ranges
        bvf_scaler = MinMaxScaler(feature_range=(0., 0.4), copy=False)
        a_mie_scaler = MinMaxScaler(feature_range=(5., 30.) * 100., copy=False)
        a_ray_scaler = MinMaxScaler(feature_range=(0., 60.) * 100., copy=False)
        d_scaler = MinMaxScaler(feature_range=(0., 2000.), copy=False)
        bvf_scaler.fit_transform(samples[:, 0])
        # saO2 is the same for each layer, since the blood "diffuses" in tissue
        samples[:, 1] = saO2
        a_mie_scaler.fit_transform(samples[:, 2])
        a_ray_scaler.fit_transform(samples[:, 3])
        d_scaler.fit_transform(samples[:, 4])
        # append as last layer
        self.layers.append(samples)

    def create_parameters(self, nr_samples):

        saO2 = self.generator(nr_samples)
        # create three layers with random samples
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)

        total_d = reduce(lambda d1, d2: d1[:, -1] + d2[:, -1], self.layers)
        desired_d = 2000. * 10 ** -6
        # "normalize" d to 2mm
        for l in self.layers:
            l[:, -1] = l[:, -1] / total_d * desired_d


def join_batches(batch_1, batch_2):
    return batch_1.join(batch_2)
