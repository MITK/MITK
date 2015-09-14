'''
Created on Sep 10, 2015

@author: wirkert
'''


'''
Created on Aug 31, 2015

@author: wirkert
'''


import os
import numpy as np
import pickle
import time
import logging
import random
import luigi

import scriptpaths as sp
from mc.tissuemodels import Colon
from mc.sim import SimWrapper, get_reflectance
from msi.io.spectrometerreader import SpectrometerReader
from msi.io.msiwriter import MsiWriter
from msi.io.msireader import MsiReader
from msi.normalize import NormalizeMean
import msi.msimanipulations as msimani

# experiment configuration
WAVELENGTHS = np.arange(450, 720, 2) * 10 ** -9
MCI_FILENAME = "./temp.mci"
MCO_FILENAME = "temp.mco"
# this path definitly needs to be adapted by you
PATH_TO_MCML = "/home/wirkert/workspace/monteCarlo/gpumcml/fast-gpumcml/"
EXEC_MCML = "gpumcml.sm_20"



class SpectrometerFile(luigi.Task):
    input_file = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.DATA_FOLDER,
                                              self.input_file))


class FilterTransmission(luigi.Task):
    input_file = luigi.Parameter()

    def requires(self):
        return SpectrometerFile(self.input_file)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            "processed_transmission" + self.input_file + ".msi"))

    def run(self):
        reader = SpectrometerReader()
        filter_transmission = reader.read(self.input().path)
        # filter high and low wavelengths
        wavelengths = filter_transmission.get_wavelengths()
        fi_image = filter_transmission.get_image()
        fi_image[wavelengths < 450 * 10 ** -9] = 0.0
        fi_image[wavelengths > 720 * 10 ** -9] = 0.0
        # filter elements farther away than +- 30nm
        name_to_float = float(os.path.splitext(self.input_file)[0])
        fi_image[wavelengths < (name_to_float - 30) * 10 ** -9] = 0.0
        fi_image[wavelengths > (name_to_float + 30) * 10 ** -9] = 0.0
        # elements < 0 are set to 0.
        fi_image[fi_image < 0.0] = 0.0
        # write it
        writer = MsiWriter(filter_transmission)
        writer.write(self.output().path)


class Batch(object):
    """summarizes a batch of simulated mc spectra"""

    def __init__(self):
        self.reflectances = np.array([])
        self.mucosa = np.array([])
        self.submucosa = np.array([])
        self.nr_photons = np.array([])
        self.wavelengths = np.array([])


class CreateSpectraTask(luigi.Task):
    batch_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.batch_prefix + "_" +
                                              str(self.batch_nr) + ".imc"))

    def run(self):
        start = time.time()
        NR_ELEMENTS_IN_BATCH = 1000
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
            # 0.01 + i * 0.04  # 0.04  # random.uniform(0.001, 0.4)
            muc_saO2 = random.uniform(0., 1.)
            # 0.7  # i * 0.1  # random.uniform(0., 1.)
            muc_dsp = random.uniform(0.001, 0.6)
            # 0.01 + i * 0.06  # 0.2  # random.uniform(0.001, 0.6)
            muc_d = random.uniform(250, 735) * 10 ** -6
            # (250 + 0.1 * i * 500) * 10 ** -6
            # random.uniform(250, 735) * 10 ** -6
            sm_bvf = random.uniform(0.03, 0.4)
            # 0.1  # random.uniform(0.03, 0.4)
            sm_saO2 = muc_saO2  # submocosa and mucosa oxygenation are equal
            sm_dsp = random.uniform(0.03, 0.6)
            # 0.2  # random.uniform(0.03, 0.6)
            # set layers to these values
            self.colon_model.set_mucosa(bvf=muc_bvf, saO2=muc_saO2,
                                        dsp=muc_dsp, d=muc_d)
            self.colon_model.set_submucosa(bvf=sm_bvf, saO2=sm_saO2,
                                        dsp=sm_dsp)
            # map the wavelengths array to reflectance list
            reflectances = map(self.wavelength_to_reflectance, WAVELENGTHS)
            # store in batch_nr
            mucosa, submucosa = self.colon_model.get_layer_parameters()
            batch.mucosa[i] = mucosa
            batch.submucosa[i] = submucosa
            batch.reflectances[i] = np.asarray(reflectances)
            # success!
            logging.info("successfully ran simulation " + str(i) + " for\n" +
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

        end = time.time()
        logging.info("time for creating batch of mc data: %.f s" % (end - start))


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


def get_transmission_data(input_path, desired_wavelengths):
    """small helper method to get filter transmission from
    file at input_path. The wavelengths are interpolated to the desired ones"""
    reader = MsiReader()
    filter_transmission = reader.read(input_path)
    msimani.interpolate_wavelengths(filter_transmission, desired_wavelengths)
    # normalize to one
    normalizer = NormalizeMean()
    normalizer.normalize(filter_transmission)
    return filter_transmission
