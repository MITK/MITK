'''
Created on Sep 10, 2015

@author: wirkert
'''



import os

import pandas as pd
import luigi

import scriptpaths as sp
import mc.dfmanipulations as dfmani
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

        # write it to a dataframe
        df = pd.DataFrame()
        df["wavelengths"] = wavelengths
        df["reflectances"] = fi_image
        df.to_csv(self.output().path, index=False)


class JoinBatches(luigi.Task):
    df_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.df_prefix + "_" +
                                              "all" + ".txt"))

    def run(self):
        path = os.path.join(sp.ROOT_FOLDER, sp.MC_DATA_FOLDER)
        # get all files in the search path
        files = [ f for f in os.listdir(path) \
                 if os.path.isfile(os.path.join(path, f)) ]
        # from these get only those who start with correct batch prefix
        df_file_names = [os.path.join(path, f) for f in files
                         if f.startswith(self.df_prefix)]
        # load these files
        dfs = [pd.read_csv(f, header=[0, 1]) for f in df_file_names]
        # now join them to one batch
        joined_df = pd.concat(dfs, ignore_index=True)
        # write it
        joined_df.to_csv(self.output().path, index=False)


class CameraBatch(luigi.Task):
    """takes a batch of reference data and converts it to the spectra
    processed by a camera with the specified wavelengths assuming a 10nm FWHM"""
    df_prefix = luigi.Parameter()

    def requires(self):
        return JoinBatches(self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.df_prefix +
                                              "_all_camera.txt"))

    def run(self):
        # load dataframe
        df = pd.read_csv(self.input().path, header=[0, 1])
        # camera batch creation:
        dfmani.fold_by_sliding_average(df, 6)
        dfmani.interpolate_wavelengths(df, sp.RECORDED_WAVELENGTHS)
        # write it
        df.to_csv(self.output().path, index=False)


class SpectroCamBatch(luigi.Task):
    """
    Same as CameraBatch in purpose but adapts the batch to our very specific
    SpectroCam set-up.
    """
    df_prefix = luigi.Parameter()

    def requires(self):
        return JoinBatches(self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              self.df_prefix +
                                              "_all_spectrocam.txt"))

    def run(self):
        pass


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
