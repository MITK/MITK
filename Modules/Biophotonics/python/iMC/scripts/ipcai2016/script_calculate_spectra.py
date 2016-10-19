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

import commons
import mc.factories as mcfac
from mc.sim import SimWrapper
from mc.create_spectrum import create_spectrum

# parameter setting
NR_BATCHES = 100
NR_ELEMENTS_IN_BATCH = 1000
# the wavelengths to be simulated
WAVELENGHTS = np.arange(450, 720, 2) * 10 ** -9
NR_PHOTONS = 10 ** 6

# experiment configuration
MCI_FILENAME = "./temp.mci"
MCO_FILENAME = "temp.mco"
# this path definitly needs to be adapted by you
PATH_TO_MCML = "/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/"
EXEC_MCML = "gpumcml.sm_20"


sc = commons.ScriptCommons()


class CreateSpectraTask(luigi.Task):
    df_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()
    nr_samples = luigi.IntParameter()
    factory = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("MC_DATA_FOLDER"),
                                              self.df_prefix + "_" +
                                              str(self.batch_nr) + ".txt"))

    def run(self):
        start = time.time()
        # setup simulation wrapper
        sim_wrapper = SimWrapper()
        sim_wrapper.set_mci_filename(MCI_FILENAME)
        sim_wrapper.set_mcml_executable(os.path.join(PATH_TO_MCML, EXEC_MCML))
        # setup model
        tissue_model = self.factory.create_tissue_model()
        tissue_model.set_mci_filename(sim_wrapper.mci_filename)
        tissue_model.set_mco_filename(MCO_FILENAME)
        tissue_model.set_nr_photons(NR_PHOTONS)
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
            # set the desired element in the dataframe to be simulated
            tissue_model.set_dataframe_row(df.loc[i, :])
            logging.info("running simulation " + str(i) + " for\n" +
                         str(tissue_model))
            reflectances = create_spectrum(tissue_model, sim_wrapper,
                                           WAVELENGHTS)
            # store in dataframe
            for r, w in zip(reflectances, WAVELENGHTS):
                df["reflectances", w][i] = r

        # clean up temporarily created files
        os.remove(MCI_FILENAME)
        created_mco_file = os.path.join(PATH_TO_MCML, MCO_FILENAME)
        if os.path.isfile(created_mco_file):
            os.remove(created_mco_file)
        # save the created output
        f = open(self.output().path, 'w')
        df.to_csv(f)

        end = time.time()
        logging.info("time for creating batch of mc data: %.f s" %
                     (end - start))


if __name__ == '__main__':

    # create a folder for the results if necessary
    sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
    sc.create_folders()

    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                 "calculate_spectra" +
                                 str(datetime.datetime.now()) +
                                 '.log'),
                        level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)
    luigi.interface.setup_interface_logging()

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    BATCH_NUMBERS = np.arange(0, NR_BATCHES, 1)
    for i in BATCH_NUMBERS:
        colon_task = CreateSpectraTask("ipcai_revision_generic_mean_scattering",
                                       i,
                                       NR_ELEMENTS_IN_BATCH,
                                       mcfac.GenericMeanScatteringFactory())
        w.add(colon_task)
        w.run()

