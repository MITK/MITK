'''
Created on Sep 9, 2015

@author: wirkert
'''


import os
import numpy as np
import pickle
import time
import logging
import random

import matplotlib.pyplot as plt
import luigi

from mc.tissuemodels import Colon
from mc.sim import SimWrapper, get_reflectance
import scriptpaths as sp


# experiment configuration
WAVELENGTHS = np.arange(450, 720, 2) * 10 ** -9
MCI_FILENAME = "/media/wirkert/data/Data/2015_xx_xx_imc/temp.mci"
MCO_FILENAME = "temp.mco"
# this path definitly needs to be adapted by you
PATH_TO_MCML = "/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/"
EXEC_MCML = "gpumcml.sm_20"
# general output path config
sp.ROOT_FOLDER = "/media/wirkert/data/Data/2015_xx_xx_imc"


class Batch(object):

    def __init__(self):
        self.reflectances = np.array([])
        self.mucosa = np.array([])
        self.submucosa = np.array([])
        self.nr_photons = np.array([])
        self.wavelengths = np.array([])


class CreateSpectraTask(luigi.Task):
    batch = luigi.IntParameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              "batch_" +
                                              str(self.batch) + ".imc"))

    def run(self):
        start = time.time()
        NR_ELEMENTS_IN_BATCH = 10000
        # setup simulation wrapper
        self.sim_wrapper = SimWrapper()
        self.sim_wrapper.set_mci_filename(MCI_FILENAME)
        self.sim_wrapper.set_mcml_executable(PATH_TO_MCML + EXEC_MCML)
        # setup colon model
        self.colon_model = Colon()
        self.colon_model.nr_photons = 10 ** 6
        self.colon_model.set_mci_filename(self.sim_wrapper.mci_filename)
        self.colon_model.set_mco_filename(MCO_FILENAME)
        # setup array in which data shall be stored
        batch = Batch()
        batch.reflectances = np.zeros((NR_ELEMENTS_IN_BATCH, len(WAVELENGTHS)))
        batch.mucosa = np.zeros((NR_ELEMENTS_IN_BATCH,
                            len(self.colon_model.get_layer_parameters()[0])))
        batch.submucosa = np.zeros((NR_ELEMENTS_IN_BATCH,
                            len(self.colon_model.get_layer_parameters()[1])))
        batch.nr_photons = self.colon_model.nr_photons
        batch.wavelengths = WAVELENGTHS
        # now generate NR_ELEMENTS_IN_BATCH different colon spectra
        for i in range(NR_ELEMENTS_IN_BATCH):
            # randomly draw paramters for submucosa and mucosa
            muc_bvf = random.uniform(0.001, 0.4)
            muc_saO2 = random.uniform(0., 1.)
            muc_dsp = random.uniform(0.001, 0.6)
            muc_d = random.uniform(250, 735) * 10 ** -6
            sm_bvf = random.uniform(0.03, 0.4)
            sm_saO2 = muc_saO2  # submocosa and mucosa oxygenation are equal
            sm_dsp = random.uniform(0.03, 0.6)
            # set layers to these values
            self.colon_model.set_mucosa(bvf=muc_bvf, saO2=muc_saO2,
                                        dsp=muc_dsp, d=muc_d)
            self.colon_model.set_submucosa(bvf=sm_bvf, saO2=sm_saO2,
                                        dsp=sm_dsp)
            # map the wavelengths array to reflectance list
            reflectances = map(self.wavelength_to_reflectance, WAVELENGTHS)
            # store in batch
            mucosa, submucosa = self.colon_model.get_layer_parameters()
            batch.mucosa[i] = mucosa
            batch.submucosa[i] = submucosa
            batch.reflectances[i] = np.asarray(reflectances)
            # success!
            logging.info("successfully ran simulation for\n" +
                         str(self.colon_model))
        # clean up temporarily created files
        os.remove(MCI_FILENAME)
        created_mco_file = PATH_TO_MCML + "/" + MCO_FILENAME
        if os.path.isfile(created_mco_file):
            os.remove(created_mco_file)
        # save the created output
        f = self.output().open('w')
        pickle.dump(batch, f)
        f.close()
#
#         plt.plot(WAVELENGTHS, reflectances, "-*")
#         plt.grid()
#         plt.show()
        end = time.time()
        print("time for batch of mc data: ", str(end - start), "s")

    def wavelength_to_reflectance(self, wavelength):
        """helper function to determine the reflectance for a given
        wavelength using the current model and simulation """
        self.colon_model.set_wavelength(wavelength)
        self.colon_model.create_mci_file()
        if os.path.isfile(PATH_TO_MCML + EXEC_MCML):
            self.sim_wrapper.run_simulation()
            return get_reflectance(PATH_TO_MCML + MCO_FILENAME)
        else:
            raise IOError("path to gpumcml not valid")


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    NR_BATCHES_TO_CREATE = 10
    for i in range(NR_BATCHES_TO_CREATE):
        main_task = CreateSpectraTask(i)
        w.add(main_task)
        w.run()


