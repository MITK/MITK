'''
Created on Oct 12, 2015

@author: wirkert
'''

import pickle
import logging

import numpy as np
import matplotlib.pyplot as plt
import luigi

import tasks_regression as rt
from msi.plot import plot
from msi.msi import Msi
import msi.normalize as norm
import scriptpaths as sp

sp.ROOT_FOLDER = "/media/wirkert/data/Data/2015_xxxx_plot_one_spectrum"

# the wavelengths recorded by our camera
RECORDED_WAVELENGTHS = \
        np.arange(450, 720, 2) * 10 ** -9
PARAMS = np.array([0.1,  # bvf
                   0.7,  # SaO2
                   0.0,  # billirubin
                   500.,  # a_mie
                   0.0,  # a_ray
                   1.091,  # b (for scattering
                   500. * 10 ** -6])  # d_muc

class PlotOneSpectrum(luigi.Task):
    df_prefix = luigi.Parameter()

    def requires(self):
        return rt.TrainForestForwardModel(self.df_prefix)


    def run(self):
        f = file(self.input().path, "r")
        rf = pickle.load(f)
        f.close()

        refl = rf.predict(PARAMS)

        msi = Msi(refl)
        msi.set_wavelengths(RECORDED_WAVELENGTHS)
        # norm.standard_normalizer.normalize(msi)

        plot(msi)
        plt.gca().set_xlabel("wavelength")
        plt.gca().set_ylabel("normalized reflectance")
        plt.grid()
        plt.ylim([0.0, 0.4])
        plt.title("bvf: " + str(PARAMS[0]) + "; saO2: " + str(PARAMS[1]) +
                  "; bili: " + str(PARAMS[2]) + "; a_mie: " + str(PARAMS[3]) +
                  "; a_ray: " + str(PARAMS[4]) + "; d_muc: " + str(PARAMS[6]))
        plt.show()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    main_task = PlotOneSpectrum(batch_prefix=
            "jacques_no_billi_generic_scattering_")
    w.add(main_task)
    w.run()
