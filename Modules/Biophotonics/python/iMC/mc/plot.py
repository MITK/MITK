'''
Created on Oct 16, 2015

@author: wirkert
'''


import numpy as np
import matplotlib.pyplot as plt


class PlotFunctor(object):
    """helping functor necessary because we need to save color for plotting"""

    def __init__(self, axes, wavelengths, nr_plot_elements):
        self.axes = axes
        self.sortedIndices = sorted(range(len(wavelengths)),
                           key=lambda k: wavelengths[k])
        self.sortedWavelenghts = wavelengths[self.sortedIndices]
        self.nr_plot_elements = nr_plot_elements
        self.i = 0


    def __call__(self, r):
        pass
        # set color so it slowly moves from blue to red
        plt_color = (1. / float(self.nr_plot_elements) * self.i,
                0.,
                1. - (1. / float(self.nr_plot_elements) * self.i))
        self.axes.plot(self.sortedWavelenghts, r[self.sortedIndices], "-o",
                       color=plt_color)
        self.i += 1
        return self.i

def plot(batch, axes=None):
    if axes is None:
        axes = plt.gca()

    f = PlotFunctor(axes, batch._wavelengths, batch.reflectances.shape[0])

    np.apply_along_axis(f,
                        axis=1,
                        arr=batch.reflectances)
