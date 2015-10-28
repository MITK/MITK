'''
Created on Oct 15, 2015

@author: wirkert
'''

import copy

import numpy as np


class AbstractBatch(object):
    """summarizes a batch of simulated mc spectra"""

    def __init__(self):
        self.reflectances = np.array([])
        self.layers = []  # list of np arrays, one for each layer
        self.nr_photons = 10 ** 6
        self.wavelengths = np.arange(450, 720, 2) * 10 ** -9
        # by default random uniform samples will be generated
        self.generator = np.random.random_sample

    def create_parameters(self, nr_samples):
        """create the parameters for the batch, the simulation has
        to create the resulting reflectances"""
        pass

    def nr_elements(self):
        return self.layers[0].shape[0]

    def join(self, other_batch):
        """helper method to join two batches"""
        joined_batch = copy.deepcopy(self)

        f = lambda x, y: np.append(x, y, axis=0)

        joined_batch.layers = map(f, self.layers, other_batch.layers)
        joined_batch.reflectances = f(joined_batch.reflectances,
                                       other_batch.reflectances)
        # next two should be the same for two batches from the same experiment
        np.testing.assert_almost_equal(joined_batch.wavelengths,
                                              other_batch.wavelengths)
        np.testing.assert_almost_equal(joined_batch.nr_photons,
                                              other_batch.nr_photons)
        return joined_batch




class GenericBatch(AbstractBatch):
    """generic three layer batch """

    def __init__(self):
        super(GenericBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini

        samples = self.generator(size=(nr_samples, 5))
        # scale samples to correct ranges
        samples[:, 0] = scale(samples[:, 0], 0., 1.00)
        # saO2 is the same for each layer, since the blood "diffuses" in tissue
        samples[:, 1] = saO2  # saO2
        samples[:, 2] = scale(samples[:, 2], 5.* 100., 30.* 100.)  # a_mie
        samples[:, 3] = scale(samples[:, 3], 0.* 100., 60.* 100.) * 0  # a_ray
        # d will be normalized later to 2mm total depth
        samples[:, 4] = scale(samples[:, 4], 0.0, 1.)  # d
        # append as last layer
        self.layers.append(samples)

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = self.generator(size=nr_samples)
        # create three layers with random samples
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)
        # create empty reflectances matrix
        self.reflectances = np.zeros((nr_samples, len(self.wavelengths)))
        # set all weights to 1
        self.weights = np.ones(nr_samples, dtype=float)

        # "normalize" d to 2mm
        ds = map(lambda x: x[:, -1], self.layers)
        total_d = reduce(lambda x, y: x + y, ds)
        desired_d = 2000. * 10 ** -6
        for l in self.layers:
            l[:, -1] = l[:, -1] / total_d * desired_d


class GaussianBatch(GenericBatch):
    """three layer batch with gaussian sampled tissue parameters"""

    def __init__(self):
        super(GaussianBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""
        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # rescale gaussian
        scale_gaussian = lambda x, u, sigma: x * sigma + u
        samples = self.generator(size=(nr_samples, 5))
        # scale samples to correct ranges
        # bvf
        samples[:, 0] = np.random.normal(size=nr_samples)
        samples[:, 0] = np.clip(scale_gaussian(samples[:, 0], 0.04, 0.02),
                                       0.,
                                       1.)
        # saO2 is the same for each layer, since the blood "diffuses" in tissue
        samples[:, 1] = scale(saO2, 0., 1.)
        # a_mie
        samples[:, 2] = scale(samples[:, 2], 5. *100, 30.* 100)
        # a_ray
        samples[:, 3] = scale(samples[:, 3], 0.*100., 60.*100.) * 0.
        # d will be normalized later to 2mm total depth
        samples[:, 4] = scale(samples[:, 4], 0., 1.)
        # append as last layer
        self.layers.append(samples)



class ColonMuscleBatch(AbstractBatch):
    """generic three layer batch """

    def __init__(self):
        super(ColonMuscleBatch, self).__init__()

    def append_one_layer(self, saO2, d_ranges, nr_samples):
        """helper function to create parameters for one layer"""
        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # rescale gaussian
        scale_gaussian = lambda x, u, sigma: x * sigma + u
        samples = self.generator(size=(nr_samples, 5))
        # scale samples to correct ranges
        # bvf
        samples[:, 0] = np.random.normal(size=nr_samples)
        samples[:, 0] = np.clip(scale_gaussian(samples[:, 0], 0.04, 0.02),
                                       0.,
                                       1.)
        # saO2 is the same for each layer, since the blood "diffuses" in tissue
        samples[:, 1] = saO2  # saO2
        samples[:, 2] = scale(samples[:, 2], 5.* 100., 30.* 100.)  # a_mie
        samples[:, 3] = scale(samples[:, 3], 0.* 100., 60.* 100.) * 0.  # a_ray
        # d
        samples[:, 4] = scale(samples[:, 4], d_ranges[0], d_ranges[1])  # d
        # append as last layer
        self.layers.append(samples)

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = self.generator(size=nr_samples)
        # create three layers with random samples
        # muscle
        self.append_one_layer(saO2, (600.*10 ** -6, 1010.*10 ** -6), nr_samples)
        # submucosa
        self.append_one_layer(saO2, (415.*10 ** -6, 847.*10 ** -6), nr_samples)
        # mucosa
        self.append_one_layer(saO2, (395.*10 ** -6, 603.*10 ** -6), nr_samples)
        # create empty reflectances matrix
        self.reflectances = np.zeros((nr_samples, len(self.wavelengths)))
        # set all weights to 1
        self.weights = np.ones(nr_samples, dtype=float)


class VisualizationBatch(AbstractBatch):
    """batch used for visualization of different spectra. Feel free to adapt
    for your visualization purposes."""

    def append_one_layer(self, bvf, saO2, a_mie, a_ray, d, nr_samples):
        """helper function to create parameters for one layer"""
        samples = np.zeros((nr_samples, 5))
        # scale samples to correct ranges
        samples[:, 0] = bvf
        samples[:, 1] = saO2
        samples[:, 2] = a_mie
        samples[:, 3] = a_ray
        # d will be normalized later to 2mm total depth
        samples[:, 4] = d
        # append as last layer
        self.layers.append(samples)

    def create_parameters(self, nr_samples):
        # bvf = np.linspace(0.0, 1., nr_samples)
        # saO2 = np.linspace(0., 1., nr_samples)
        # d = np.linspace(175, 735, nr_samples) * 10 ** -6
        # a_mie = np.linspace(5., 30., nr_samples) * 100
        a_ray = np.linspace(0., 60., nr_samples) * 100
        # create three layers with random samples
        self.append_one_layer(0.04, 0.7, 30.*100., a_ray, 500 * 10 ** -6,
                              nr_samples)
        self.append_one_layer(0.04, 0.7, 5.0 * 100, 0.*100, 500 * 10 ** -6,
                              nr_samples)
        self.append_one_layer(0.04, 0.7, 5.0 * 100, 0.*100, 500 * 10 ** -6,
                              nr_samples)
        # create empty reflectances matrix
        self.reflectances = np.zeros((nr_samples, len(self.wavelengths)))
        # set all weights to 1
        self.weights = np.ones(nr_samples, dtype=float)


def join_batches(batch_1, batch_2):
    return batch_1.join(batch_2)
