'''
Created on Oct 15, 2015

@author: wirkert
'''

import numpy as np
from pandas import DataFrame
import pandas as pd


class AbstractBatch(object):
    """summarizes a batch of simulated mc spectra"""

    def __init__(self):
        self._nr_layers = 0  # internally keeps track of number of layers
        my_index = pd.MultiIndex(levels=[[], []],
                             labels=[[], []])
        self.df = DataFrame(columns=my_index)

    def create_parameters(self, nr_samples):
        """create the parameters for the batch, the simulation has
        to create the resulting reflectances"""
        pass

    def nr_elements(self):
        return self.df.shape[0]


class GenericBatch(AbstractBatch):
    """generic n-layer batch with each layer having the same oxygenation """

    def __init__(self):
        super(GenericBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # shortcut to random generator
        gen = np.random.random_sample
        gen_n = np.random.normal

        # create layer elements
        self.df["layer" + str(self._nr_layers), "vhb"] = \
            scale(gen(nr_samples), 0, 1.)
        self.df["layer" + str(self._nr_layers), "sao2"] = \
            saO2
        self.df["layer" + str(self._nr_layers), "a_mie"] = \
            np.clip(gen_n(loc=18.9, scale=10.2, size=nr_samples),
                    0.1, np.inf) * 100  # to 1/m
        self.df["layer" + str(self._nr_layers), "b_mie"] = \
            np.clip(gen_n(loc=1.286, scale=0.521, size=nr_samples), 0, np.inf)
        self.df["layer" + str(self._nr_layers), "d"] = \
            scale(gen(nr_samples), 0, 1.)
        self.df["layer" + str(self._nr_layers), "n"] = \
            scale(gen(nr_samples), 1.33, 1.54)
        self.df["layer" + str(self._nr_layers), "g"] = \
            scale(gen(nr_samples), 0.8, 0.95)
        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = np.random.random_sample(size=nr_samples)

        # create three layers with random samples
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)

        # "normalize" d to 2mm
        # first extract all layers from df
        self.df

        layers = [l for l in self.df.columns.levels[0] if "layer" in l]
        # summarize all ds
        sum_d = 0
        for l in layers:
            sum_d += self.df[l, "d"]
        for l in layers:
            self.df[l, "d"] = self.df[l, "d"] / sum_d * 2000. * 10 ** -6
            self.df[l, "d"] = np.clip(self.df[l, "d"], 25 * 10 ** -6, np.inf)

        return self.df


class GenericBatch(AbstractBatch):
    """generic n-layer batch with each layer having the same oxygenation """

    def __init__(self):
        super(GenericBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # shortcut to random generator
        gen = np.random.random_sample
        gen_n = np.random.normal

        # create layer elements
        self.df["layer" + str(self._nr_layers), "vhb"] = \
            scale(gen(nr_samples), 0, 1.)
        self.df["layer" + str(self._nr_layers), "sao2"] = \
            saO2
        self.df["layer" + str(self._nr_layers), "a_mie"] = \
            np.clip(gen_n(loc=18.9, scale=10.2, size=nr_samples),
                    0.1, np.inf) * 100  # to 1/m
        self.df["layer" + str(self._nr_layers), "b_mie"] = \
            np.clip(gen_n(loc=1.286, scale=0.521, size=nr_samples), 0, np.inf)
        self.df["layer" + str(self._nr_layers), "d"] = \
            scale(gen(nr_samples), 0, 1.)
        self.df["layer" + str(self._nr_layers), "n"] = \
            scale(gen(nr_samples), 1.33, 1.54)
        self.df["layer" + str(self._nr_layers), "g"] = \
            scale(gen(nr_samples), 0.8, 0.95)
        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = np.random.random_sample(size=nr_samples)

        # create three layers with random samples
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)
        self.append_one_layer(saO2, nr_samples)

        # "normalize" d to 2mm
        # first extract all layers from df
        self.df

        layers = [l for l in self.df.columns.levels[0] if "layer" in l]
        # summarize all ds
        sum_d = 0
        for l in layers:
            sum_d += self.df[l, "d"]
        for l in layers:
            self.df[l, "d"] = self.df[l, "d"] / sum_d * 2000. * 10 ** -6
            self.df[l, "d"] = np.clip(self.df[l, "d"], 25 * 10 ** -6, np.inf)

        return self.df


class LessGenericBatch(AbstractBatch):
    """less generic three layer batch. This only varies blood volume fraction
    w.r.t. the ColonMuscleBatch. Let's see if DA works in this case."""

    def __init__(self):
        super(LessGenericBatch, self).__init__()

    def append_one_layer(self, saO2, n, d_ranges, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # shortcut to random generator
        gen = np.random.random_sample

        # create as generic batch
        super(LessGenericBatch, self).append_one_layer(saO2, nr_samples)
        self._nr_layers -= 1  # we're not finished

        # but some changes in specific layer elements
        # more specific layer thicknesses
        self.df["layer" + str(self._nr_layers), "d"] = \
            scale(gen(nr_samples), d_ranges[0], d_ranges[1])
        # more specific n
        self.df["layer" + str(self._nr_layers), "n"] = \
            n

        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = np.random.random_sample(size=nr_samples)
        n = np.ones_like(saO2)
        # create three layers with random samples
        # muscle
        self.append_one_layer(saO2, n * 1.36, (600.*10 ** -6, 1010.*10 ** -6),
                              nr_samples)
        # submucosa
        self.append_one_layer(saO2, n * 1.36, (415.*10 ** -6, 847.*10 ** -6),
                              nr_samples)
        # mucosa
        self.append_one_layer(saO2, n * 1.38, (395.*10 ** -6, 603.*10 ** -6),
                              nr_samples)

        return self.df


class ColonMuscleBatch(GenericBatch):
    """three layer batch simulating colonic tissue"""

    def __init__(self):
        super(ColonMuscleBatch, self).__init__()

    def append_one_layer(self, saO2, n, d_ranges, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # shortcut to random generator
        gen = np.random.random_sample

        # create as generic batch
        super(ColonMuscleBatch, self).append_one_layer(saO2, nr_samples)
        self._nr_layers -= 1  # we're not finished

        # but some changes in specific layer elements
        # less blood
        self.df["layer" + str(self._nr_layers), "vhb"] = \
            scale(gen(nr_samples), 0, 0.1)
        # more specific layer thicknesses
        self.df["layer" + str(self._nr_layers), "d"] = \
            scale(gen(nr_samples), d_ranges[0], d_ranges[1])
        # more specific n
        self.df["layer" + str(self._nr_layers), "n"] = \
            n

        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        """Create generic three layer batch with a total diameter of 2mm.
        saO2 is the same in all layers, but all other parameters vary randomly
        within each layer"""
        saO2 = np.random.random_sample(size=nr_samples)
        n = np.ones_like(saO2)
        # create three layers with random samples
        # muscle
        self.append_one_layer(saO2, n * 1.36, (600.*10 ** -6, 1010.*10 ** -6),
                              nr_samples)
        # submucosa
        self.append_one_layer(saO2, n * 1.36, (415.*10 ** -6, 847.*10 ** -6),
                              nr_samples)
        # mucosa
        self.append_one_layer(saO2, n * 1.38, (395.*10 ** -6, 603.*10 ** -6),
                              nr_samples)

        return self.df


class GenericMeanScatteringBatch(GenericBatch):
    """three layer batch simulating colonic tissue"""

    def __init__(self):
        super(GenericMeanScatteringBatch, self).__init__()

    def append_one_layer(self, saO2, nr_samples):
        """helper function to create parameters for one layer"""

        # create as generic batch
        super(GenericMeanScatteringBatch, self).append_one_layer(saO2,
                                                                 nr_samples)
        self._nr_layers -= 1  # we're not finished

        # restrict exponential scattering to mean value for soft tissue.
        self.df["layer" + str(self._nr_layers), "b_mie"] = 1.286

        self._nr_layers += 1


class ColonMuscleMeanScatteringBatch(ColonMuscleBatch):
    """three layer batch simulating colonic tissue"""

    def __init__(self):
        super(ColonMuscleMeanScatteringBatch, self).__init__()

    def append_one_layer(self, saO2, n, d_ranges, nr_samples):
        """helper function to create parameters for one layer"""

        # create as generic batch
        super(ColonMuscleMeanScatteringBatch, self).append_one_layer(saO2,
                                                                     n,
                                                                     d_ranges,
                                                                     nr_samples)
        self._nr_layers -= 1  # we're not finished

        # restrict exponential scattering to mean value for soft tissue.
        self.df["layer" + str(self._nr_layers), "b_mie"] = 1.286

        self._nr_layers += 1


class VisualizationBatch(AbstractBatch):
    """batch used for visualization of different spectra. Feel free to adapt
    for your visualization purposes."""

    def __init__(self):
        super(VisualizationBatch, self).__init__()

    def append_one_layer(self, vhb, sao2, a_mie, b_mie, d, n, g, nr_samples):
        """helper function to create parameters for one layer"""

        # create layer elements
        self.df["layer" + str(self._nr_layers), "vhb"] = vhb
        self.df["layer" + str(self._nr_layers), "sao2"] = sao2
        self.df["layer" + str(self._nr_layers), "a_mie"] = a_mie
        self.df["layer" + str(self._nr_layers), "b_mie"] = b_mie
        self.df["layer" + str(self._nr_layers), "d"] = d
        self.df["layer" + str(self._nr_layers), "n"] = n
        self.df["layer" + str(self._nr_layers), "g"] = g
        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        # bvf = np.linspace(0.0, .1, nr_samples)
        # saO2 = np.linspace(0., 1., nr_samples)
        # d = np.linspace(175, 735, nr_samples) * 10 ** -6
        # a_mie = np.linspace(5., 30., nr_samples) * 100
        # a_ray = np.linspace(0., 60., nr_samples) * 100
        # n = np.linspace(1.33, 1.54, nr_samples)
        # g = np.linspace(0, 0.95, nr_samples)
        # create three layers with random samples
        self.append_one_layer([0.1, 0.02], [0.7, 0.1], 18.9*100., 1.286,
                              500 * 10 ** -6, 1.38, 0.9,
                              nr_samples)
        self.append_one_layer(0.04, 0.7, 18.9*100., 1.286, 500 * 10 ** -6,
                              1.36, 0.9,
                              nr_samples)
        self.append_one_layer(0.04, 0.7, 18.9*100., 1.286, 500 * 10 ** -6,
                              1.36, 0.9,
                              nr_samples)

        return self.df


class IntralipidPhantomBatch(AbstractBatch):
    """batch used for visualization of different spectra. Feel free to adapt
    for your visualization purposes."""

    def __init__(self):
        super(IntralipidPhantomBatch, self).__init__()

    def append_one_layer(self, nr_samples):
        """helper function to create parameters for one layer"""

        # scales data to lie between maxi and mini instead of 0 and 1
        scale = lambda x, mini, maxi: x * (maxi - mini) + mini
        # shortcut to random generator
        gen = np.random.random_sample

        # create layer elements
        self.df["layer" + str(self._nr_layers), "vhb"] = \
            scale(gen(nr_samples), 0.001, 0.1)
        self.df["layer" + str(self._nr_layers), "sao2"] = \
            scale(gen(nr_samples), 0., 1.)
        self.df["layer" + str(self._nr_layers), "a_mie"] = \
            scale(gen(nr_samples), 5., 40.) * 100  # to 1/m
        self.df["layer" + str(self._nr_layers), "b_mie"] = \
            scale(gen(nr_samples), 2.3, 2.4)
        self.df["layer" + str(self._nr_layers), "d"] = \
            2000.*10**-6
        self.df["layer" + str(self._nr_layers), "n"] = \
            scale(gen(nr_samples), 1.33, 1.54)
        self.df["layer" + str(self._nr_layers), "g"] = \
            scale(gen(nr_samples), 0.8, 0.95)
        self._nr_layers += 1

    def create_parameters(self, nr_samples):
        """Create intralipid batch with a total diameter of 2mm.
        all other parameters vary randomly
        within each layer to simulate the interlipid scattering/absorption
        properties."""

        # create three layers with random samples
        self.append_one_layer(nr_samples)

        return self.df
