'''
Created on Sep 10, 2015

@author: wirkert
'''


'''
Created on Aug 31, 2015

@author: wirkert
'''


import os
import pickle
import luigi

import scriptpaths as sp
import mc.mcmanipulations as mcmani
from msi.io.spectrometerreader import SpectrometerReader
from msi.io.msiwriter import MsiWriter
from msi.io.msireader import MsiReader
from msi.normalize import NormalizeMean
import msi.msimanipulations as msimani





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
        # filter high and low _wavelengths
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
        # there seems to be a bug in pickle. thus, unfortunately the _generator
        # has to be removed before saving
        joined_batch._generator = None
        pickle.dump(joined_batch, joined_batch_file)


class CameraBatch(luigi.Task):
    """takes a batch of reference data and converts it to the spectra
    processed by the camera"""
    batch_prefix = luigi.Parameter()

    def requires(self):
        return JoinBatches(self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            self.batch_prefix + "_all_camera.imc"))

    def run(self):
        f = file(self.input().path, "r")
        batch = pickle.load(f)
        f.close()
        # camera batch creation:
        camera_batch = batch
        mcmani.fold_by_sliding_average(camera_batch, 6)
        mcmani.interpolate_wavelengths(camera_batch,
                                       sp.RECORDED_WAVELENGTHS)
        # write it
        joined_batch_file = open(self.output().path, 'w')
        pickle.dump(camera_batch, joined_batch_file)




def get_transmission_data(input_path, desired_wavelengths):
    """small helper method to get filter transmission from
    file at input_path. The _wavelengths are interpolated to the desired ones"""
    reader = MsiReader()
    filter_transmission = reader.read(input_path)
    msimani.interpolate_wavelengths(filter_transmission, desired_wavelengths)
    # normalize to one
    normalizer = NormalizeMean()
    normalizer.normalize(filter_transmission)
    return filter_transmission
