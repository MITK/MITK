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
import tasks_mc
import mc.plot
import mc.factories as mcfac


# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_06_01_Filtertransmittance_Spectrometer"
sp.DATA_FOLDER = "spectrometer_measurements"
sp.MC_DATA_FOLDER = "processed"
SPECTRAL_PLOTS = "spectral_plots"


class VisualizeSpectraTask(luigi.Task):
    batch_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()
    nr_samples = luigi.IntParameter()

    def requires(self):
        return tasks_mc.CreateSpectraTask(self.batch_prefix, self.batch_nr,
                                    self.nr_samples,
                                    mcfac.VisualizationMcFactory()), \
               tasks_mc.CameraBatch(self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              SPECTRAL_PLOTS,
            "plotted_spectra_" +
            self.batch_prefix + "_" + str(self.batch_nr) + ".png"))

    def run(self):
        f = file(self.input()[0].path, "r")
        batch = pickle.load(f)
        f = file(self.input()[1].path, "r")
        camera_batch = pickle.load(f)
        f.close()

        f, axarr = plt.subplots(2, 1)
        # axis to plot the original data c.f. the mcml simulation
        org_ax = axarr[0]
        # axis for plotting the folded spectrum + normalization
        fold_ax = axarr[1]
        mc.plot.plot(batch, org_ax)
        mc.plot.plot(camera_batch, fold_ax)

        # tidy up and save plot
        major_ticks = np.arange(450, 720, 50) * 10 ** -9
        minor_ticks = np.arange(450, 720, 10) * 10 ** -9
        org_ax.set_title(self.batch_prefix)
        org_ax.set_xticks(major_ticks)
        org_ax.set_xticks(minor_ticks, minor=True)
        fold_ax.set_title("response to camera filter sensitivities")
        fold_ax.set_xticks(major_ticks)
        fold_ax.set_xticks(minor_ticks, minor=True)
        org_ax.grid()
        fold_ax.grid()
        plt.savefig(self.output().path, dpi=500)



if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    main_task = VisualizeSpectraTask("vizualization_generic",
                                     0, 10)
    w.add(main_task)
    w.run()

