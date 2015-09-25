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
import luigi
import copy
import random
import itertools

import scriptpaths as sp
from mc.tissuemodels import ColonRowe, ColonJacques
from mc.sim import SimWrapper, get_diffuse_reflectance
from msi.io.spectrometerreader import SpectrometerReader
from msi.io.msiwriter import MsiWriter
from msi.io.msireader import MsiReader
from msi.normalize import NormalizeMean
import msi.msimanipulations as msimani
from msi.msi import Msi
from msi.normalize import standard_normalizer

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
                                              sp.SPECTROMETER_FOLDER,
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



def join_batches(batch_1, batch_2):
    """helper method to join two batches"""
    joined_batch = copy.copy(batch_1)
    np.append(joined_batch.reflectances, batch_2.reflectances, axis=0)
    np.append(joined_batch.mucosa, batch_2.mucosa, axis=0)
    np.append(joined_batch.submucosa, batch_2.submucosa, axis=0)
    # next two should be the same for two batches from the same experiment
    np.testing.assert_almost_equal(joined_batch.wavelengths,
                                          batch_2.wavelengths)
    np.testing.assert_almost_equal(joined_batch.nr_photons,
                                          batch_2.nr_photons)
    return joined_batch


class JoinBatches(luigi.Task):
    batch_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.batch_prefix + "_" +
                                              "all" + ".imc"))

    def run(self):
        path = os.path.join(sp.ROOT_FOLDER, sp.MC_DATA_FOLDER)
        # get all files in the search path
        files = [ f for f in os.listdir(path) \
                 if os.path.isfile(os.path.join(path, f)) ]
        # from these get only those who start with correct batch prefix
        batch_file_names = \
                [ f for f in files if f.startswith(self.batch_prefix)]
        batch_files = \
                [open(os.path.join(path, f), 'r') for f in batch_file_names]
        # load these files
        batches = [pickle.load(f) for f in batch_files]
        # now join them to one batch
        joined_batch = reduce(join_batches, batches)
        # write it
        joined_batch_file = open(self.output().path, 'w')
        pickle.dump(joined_batch, joined_batch_file)


class CameraBatch(luigi.Task):
    """takes a batch of reference data and converts it to the spectra
    processed by the camera"""
    batch_prefix = luigi.Parameter()

    def requires(self):
        return FilterTransmission("580.txt"), \
            FilterTransmission("470.txt"), \
            FilterTransmission("660.txt"), \
            FilterTransmission("560.txt"), \
            FilterTransmission("480.txt"), \
            FilterTransmission("511.txt"), \
            FilterTransmission("600.txt"), \
            FilterTransmission("700.txt"), \
            JoinBatches(self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            self.batch_prefix + "_all_camera.imc"))

    def run(self):
        f = file(self.input()[-1].path, "r")
        batch = pickle.load(f)
        f.close()
        # folded reflectances are of shape: nr_sampels x nr_camera_wavelengths
        folded_reflectance = np.zeros((batch.reflectances.shape[0],
                                       len(sp.RECORDED_WAVELENGTHS)))
        for j in range(len(sp.RECORDED_WAVELENGTHS)):
            filter_transmission = get_transmission_data(
                                                    self.input()[j].path,
                                                    WAVELENGTHS)
            # build weighted sum of absorption and filter transmission
            folded_reflectance[:, j] = np.sum(
                                        filter_transmission.get_image() *
                                        batch.reflectances, axis=1)
        # put the correctly folded reflectances in a temporary msi to
        # be able to use normalization
        folded_reflectance_image = Msi()
        folded_reflectance_image.set_image(folded_reflectance)
        folded_reflectance_image.set_wavelengths(sp.RECORDED_WAVELENGTHS)
        standard_normalizer.normalize(folded_reflectance_image)
        # camera batch creation:
        camera_batch = batch
        camera_batch.wavelengths = sp.RECORDED_WAVELENGTHS
        camera_batch.reflectances = folded_reflectance_image.get_image()
        # write it
        joined_batch_file = open(self.output().path, 'w')
        pickle.dump(camera_batch, joined_batch_file)


class CreateSpectraTask(luigi.Task):
    batch_prefix = luigi.Parameter()
    batch_nr = luigi.IntParameter()
    nr_samples = luigi.IntParameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.batch_prefix + "_" +
                                              str(self.batch_nr) + ".imc"))

    def run(self):
        start = time.time()
        # setup simulation wrapper
        self.sim_wrapper = SimWrapper()
        self.sim_wrapper.set_mci_filename(MCI_FILENAME)
        self.sim_wrapper.set_mcml_executable(PATH_TO_MCML + EXEC_MCML)
        # setup colon model
        self.colon_model = ColonJacques()
        self.colon_model.nr_photons = 10 ** 6
        self.colon_model.set_mci_filename(self.sim_wrapper.mci_filename)
        self.colon_model.set_mco_filename(MCO_FILENAME)
        # setup array in which data shall be stored
        batch = Batch()
        batch.reflectances = []
        batch.mucosa = []
        batch.submucosa = []
        batch.wavelengths = WAVELENGTHS

        # use this to create plot from Jacques Fig 17,
        # Optical properties of biological tissues: a review
#         a_mie_range = np.linspace(5., 20., 4)
#         a_ray_1 = np.zeros_like(a_mie_range)
#         a_ray_range = np.linspace(5., 60., 7)
#         a_mie_2 = np.ones_like(a_ray_range) * 20.
#         plt_range = np.concatenate([zip(a_mie_range, a_ray_1),
#                                    zip(a_mie_2, a_ray_range)])

        # use this to create a certain range of parameters
        bvf_ranges = np.linspace(0.01, 0.4, self.nr_samples)
        saO2_ranges = np.linspace(0.0, 1., self.nr_samples)
        # beta carotin:
        bc_ranges = np.linspace(0.0, 50., self.nr_samples)
        a_mie_ranges = np.linspace(1.0 / 100., 10. / 100.,
                                   self.nr_samples) * 100.
        a_ray_ranges = np.linspace(1., 60., self.nr_samples) * 100.
        d_ranges = np.linspace(250, 735, self.nr_samples) * 10 ** -6
        bvf_ranges = np.array([0.04])
        saO2_ranges = np.array([0.7])
        bc_ranges = np.array([20.8])
        a_mie_ranges = np.array([5.0]) * 100.
        a_ray_ranges = np.array([0.0]) * 100.
        bvf_sm_ranges = np.array([0.1])
        dsp_sm_range = np.array([0.3])
        d_ranges = np.array([500]) * 10 ** -6
        plt_range = itertools.product(bvf_ranges,
                                      saO2_ranges,
                                      bc_ranges,
                                      a_mie_ranges,
                                      a_ray_ranges,
                                      d_ranges,
                                      bvf_sm_ranges, dsp_sm_range)

        for i, (bvf, saO2, bc, a_mie, a_ray, d, bvf_sm, dsp_sm) \
                                                    in enumerate(plt_range):

            self.colon_model.set_mucosa(bvf=bvf, saO2=saO2, bc=bc,
                                        a_mie=a_mie,
                                        a_ray=a_ray,
                                        d=d)
            self.colon_model.set_submucosa(bvf=bvf_sm, saO2=saO2,
                                        dsp=dsp_sm)
            # map the wavelengths array to reflectance list
            reflectances = map(self.wavelength_to_reflectance, WAVELENGTHS)
            # store in batch_nr
            mucosa, submucosa = self.colon_model.get_layer_parameters()
            batch.mucosa.append(mucosa)
            batch.submucosa.append(submucosa)
            batch.reflectances.append(np.asarray(reflectances))
            # success!
            logging.info("successfully ran simulation " + str(i) + " for\n" +
                         str(self.colon_model))
        # convert the lists in batch to np arrays
        batch.mucosa = np.array(batch.mucosa)
        batch.submucosa = np.array(batch.submucosa)
        batch.reflectances = np.array(batch.reflectances)
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
            return get_diffuse_reflectance(PATH_TO_MCML + MCO_FILENAME)
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
