'''
Created on Sep 9, 2015

@author: wirkert
'''


import logging
import datetime
import os
import time

import numpy as np
import luigi

import scriptpaths as sp
import mc.factories as mcfac
from mc.sim import SimWrapper, get_diffuse_reflectance

# parameter setting
NR_BATCHES = 100
NR_ELEMENTS_IN_BATCH = 1000
# the wavelengths to be simulated
WAVELENGHTS = np.arange(450, 720, 2) * 10 ** -9
NR_PHOTONS = 10 ** 6

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2016_02_22_IPCAI_revision_in_silico"
sp.RESULTS_FOLDER = "mc_data"

# experiment configuration
MCI_FILENAME = "./temp.mci"
MCO_FILENAME = "temp.mco"
# this path definitly needs to be adapted by you
PATH_TO_MCML = "/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/"
EXEC_MCML = "gpumcml.sm_20"


class CreateSpectraTask(luigi.Task):
    df_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()
    nr_samples = luigi.IntParameter()
    factory = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.df_prefix + "_" +
                                              str(self.batch_nr) + ".txt"))

    def run(self):
        start = time.time()
        # setup simulation wrapper
        self.sim_wrapper = SimWrapper()
        self.sim_wrapper.set_mci_filename(MCI_FILENAME)
        self.sim_wrapper.set_mcml_executable(PATH_TO_MCML + EXEC_MCML)
        # setup model
        self.tissue_model = self.factory.create_tissue_model()
        self.tissue_model.set_mci_filename(self.sim_wrapper.mci_filename)
        self.tissue_model.set_mco_filename(MCO_FILENAME)
        self.tissue_model.set_nr_photons(NR_PHOTONS)
        # setup array in which data shall be stored
        batch = self.factory.create_batch_to_simulate()
        batch.create_parameters(self.nr_samples)
        # dataframe created by batch:
        df = batch.df
        # add reflectance column to dataframe
        for w in WAVELENGHTS:
            df["reflectances", w] = np.NAN

        # for each instance of our tissue model
        for i in range(df.shape[0]):
            self.tissue_model.set_dataframe_element(df, i)
            logging.info("running simulation " + str(i) + " for\n" +
                         str(self.tissue_model))
            start = time.time()
            # map the _wavelengths array to reflectance list
            reflectances = map(self.wavelength_to_reflectance,
                               WAVELENGHTS)
            end = time.time()
            # store in dataframe
            for r, w in zip(reflectances, WAVELENGHTS):
                df["reflectances", w][i] = r
            # success!
            logging.info("successfully ran simulation in " +
                         "{:.2f}".format(end - start) + " seconds")
        # clean up temporarily created files
        os.remove(MCI_FILENAME)
        created_mco_file = PATH_TO_MCML + "/" + MCO_FILENAME
        if os.path.isfile(created_mco_file):
            os.remove(created_mco_file)
        # save the created output
        f = open(self.output().path, 'w')
        df.to_csv(f)

        end = time.time()
        logging.info("time for creating batch of mc data: %.f s" %
                     (end - start))

    def wavelength_to_reflectance(self, wavelength):
        """helper function to determine the reflectance for a given
        wavelength using the current model and simulation """
        self.tissue_model.set_wavelength(wavelength)
        self.tissue_model.create_mci_file()
        if os.path.isfile(PATH_TO_MCML + EXEC_MCML):
            self.sim_wrapper.run_simulation()
            return get_diffuse_reflectance(PATH_TO_MCML + MCO_FILENAME)
        else:
            raise IOError("path to gpumcml not valid")


if __name__ == '__main__':
    logging.basicConfig(filename='log/calculate_spectra_' +
                        str(datetime.datetime.now()) +
                        '.log', level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)
    luigi.interface.setup_interface_logging()

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    BATCH_NUMBERS = np.arange(0, NR_BATCHES, 1)
    for i in BATCH_NUMBERS:
        colon_task = CreateSpectraTask("ipcai_revision_colon_train", i,
                                       NR_ELEMENTS_IN_BATCH,
                                       mcfac.ColonMuscleMcFactory())
        generic_task = CreateSpectraTask("ipcai_revision_generic", i,
                                         NR_ELEMENTS_IN_BATCH,
                                         mcfac.GenericMcFactory())
        w.add(colon_task)
        w.add(generic_task)
        w.run()

