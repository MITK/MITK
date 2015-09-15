'''
Created on Sep 14, 2015

@author: wirkert
'''

import logging
import os
import numpy as np

import matplotlib.pyplot as plt
import luigi

import scriptpaths as sp
from msi.io.nrrdreader import NrrdReader
import msi.msimanipulations as msimani
import msi.imgmani as imgmani

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_09_14_Linearity"
LINEARITY_IMAGE_PREFIX = "ModeSettingsLinearity"


class LinearityImage(luigi.Task):
    multiplicator = luigi.FloatParameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              LINEARITY_IMAGE_PREFIX +
                                              str(self.multiplicator) +
                                              ".nrrd"))


class Segmentation(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              "segmentation3.nrrd"))


class PlotLinearities(luigi.Task):
    linearity_factors = np.arange(0.1, 1.1, 0.1)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              LINEARITY_IMAGE_PREFIX +
                                              "_summary.png"))

    def requires(self):
        localTargets = [LinearityImage(f) for f in self.linearity_factors]
        return tuple(localTargets) + (Segmentation(),)

    def run(self):
        # get images from list
        reader = NrrdReader()
        images = [reader.read(f.path) for f in self.input()]
        linearity_images = images[0:-1]
        segmentation = images[-1]
        # apply segmentation and collapse image for better later processing
        for l in linearity_images:
            msimani.apply_segmentation(l, segmentation)
            l.set_image(imgmani.collapse_image(l.get_image()))
        # plot each wavelength
        plt.figure()
        for wav in range(linearity_images[0].get_image().shape[-1]):
            # one curve for all linearity factors
            mean_curve = [np.mean(l.get_image()[:, wav], axis=0)
                          for l in linearity_images]
            std_curve = [np.std(l.get_image()[:, wav], axis=0)
                          for l in linearity_images]
            print("we have a % std of: ", \
                  str(100 * np.array(std_curve) / np.array(mean_curve)))
            plt.errorbar(self.linearity_factors, mean_curve,
                         yerr=std_curve, fmt='-*')
        plt.savefig(self.output().path, dpi=500, bbox_inches='tight')



if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    linearity_factors = np.arange(0.1, 1.0, 0.1)
    main_task = PlotLinearities()
    w.add(main_task)
    w.run()
