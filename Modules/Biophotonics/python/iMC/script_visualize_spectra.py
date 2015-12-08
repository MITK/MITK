'''
Created on Sep 10, 2015

@author: wirkert
'''


import logging
import os
import pickle
import numpy as np

import matplotlib.pyplot as plt
import matplotlib

import luigi

from regression.preprocessing import preprocess2, normalize
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
sp.RECORDED_WAVELENGTHS = np.arange(470, 690, 10) * 10 ** -9

# matplotlib.rcParams.update({'font.size': 18})

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

        batch.wavelengths *= 10 ** 9
        # X, y = preprocess2(camera_batch, w_percent=0.1)
        # camera_batch.reflectances = X

        # f, axarr = plt.subplots(2, 1)
        # axis to plot the original data c.f. the mcml simulation

        # axis for plotting the folded spectrum + normalization

        mc.plot.plot(batch)

        # tidy up and save plot
        plt.grid()
        plt.ylabel("r", fontsize=30)
        plt.xlabel("$\lambda$ [nm]", fontsize=30)
        # org_ax.axes.xaxis.set_ticklabels([])
        # org_ax.xaxis.set_visible(False)
        plt.show()
        # plt.savefig(self.output().path, dpi=250)



if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    main_task = VisualizeSpectraTask("paper2",
                                     0, 1)
    w.add(main_task)
    w.run()

