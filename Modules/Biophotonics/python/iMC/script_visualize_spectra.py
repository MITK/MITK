'''
Created on Sep 10, 2015

@author: wirkert
'''


import logging
import os
import pickle
import numpy as np

import matplotlib.pyplot as plt

import luigi

import scriptpaths as sp
import tasks_mc as mc
from tasks_mc import WAVELENGTHS
import msi.plot
from msi.msi import Msi
import msi.normalize as norm

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_06_01_Filtertransmittance_Spectrometer"
sp.FINALS_FOLDER = "ConvolvedSpectra"
sp.DATA_FOLDER = "spectrometer_measurements"

# the wavelengths recorded by our camera
RECORDED_WAVELENGTHS = \
        np.array([580, 470, 660, 560, 480, 511, 600, 700]) * 10 ** -9





class VisualizeSpectraTask(luigi.Task):
    batch_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()

    def requires(self):
        return mc.FilterTransmission("580.txt"), \
            mc.FilterTransmission("470.txt"), \
            mc.FilterTransmission("660.txt"), \
            mc.FilterTransmission("560.txt"), \
            mc.FilterTransmission("480.txt"), \
            mc.FilterTransmission("511.txt"), \
            mc.FilterTransmission("600.txt"), \
            mc.FilterTransmission("700.txt"), \
            mc.CreateSpectraTask(self.batch_prefix, self.batch_nr)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            "plotted_spectra_" +
            self.batch_prefix + "_" + str(self.batch_nr) + ".png"))

    def run(self):
        f = file(self.input()[8].path, "r")
        batch = pickle.load(f)
        f.close()
        f, axarr = plt.subplots(1, 2)

        for i in range(batch.reflectances.shape[0]):
            axarr[0].plot(batch.wavelengths, batch.reflectances[i])
            plt.grid()
            folded_reflectance = np.zeros_like(RECORDED_WAVELENGTHS)
            for j in range(len(RECORDED_WAVELENGTHS)):
                filter_transmission = mc.get_transmission_data(
                                                        self.input()[j].path,
                                                        WAVELENGTHS)
                # build weighted sum of absorption and filter transmission
                folded_reflectance[j] = np.sum(
                                            filter_transmission.get_image() *
                                            batch.reflectances[i, :])
            folded_reflectance_image = Msi()
            folded_reflectance_image.set_image(folded_reflectance)
            folded_reflectance_image.set_wavelengths(RECORDED_WAVELENGTHS)
            normalizer = norm.NormalizeMean()
            normalizer.normalize(folded_reflectance_image)
            msi.plot.plot(folded_reflectance_image, axarr[1])

        # tidy up and save plot
        major_ticks = np.arange(450, 720, 50) * 10 ** -9
        minor_ticks = np.arange(450, 720, 10) * 10 ** -9
        axarr[0].set_xticks(major_ticks)
        axarr[0].set_xticks(minor_ticks, minor=True)
        axarr[1].set_xticks(major_ticks)
        axarr[1].set_xticks(minor_ticks, minor=True)
        axarr[0].grid()
        axarr[1].grid()
        plt.savefig(self.output().path, dpi=500)



if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    main_task = VisualizeSpectraTask("d_batch", 0)
    w.add(main_task)
    w.run()

