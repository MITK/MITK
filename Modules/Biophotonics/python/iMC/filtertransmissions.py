'''
Created on Aug 31, 2015

@author: wirkert
'''


import os
import numpy as np

import luigi
import matplotlib.pyplot as plt

from msi.msi import Msi
import scriptpaths as sp
from msi.io.spectrometerreader import SpectrometerReader
from msi.io.msiwriter import MsiWriter
from msi.io.msireader import MsiReader
from msi.normalize import NormalizeMean
import msi.msimanipulations as msimani
import msi.plot as msiPlt

sp.FINALS_FOLDER = "ConvolvedSpectra"
sp.ROOT_FOLDER = \
    "/media/wirkert/data/Data/2015_06_01_Filtertransmittance_Spectrometer/"
sp.FAP_IMAGE_FOLDER = "spectrometer_measurements"
RECORDED_WAVELENGTHS = \
        np.array([580, 470, 660, 560, 480, 511, 600, 700]) * 10 ** -9


def get_extinction_coefficients(reference_filename):
    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt(reference_filename, skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:, 0] = haemoLUT[:, 0] * 10 ** -9
    haemoLUT[:, 1:] = haemoLUT[:, 1:] * 10 ** 2
    # get the data into Msis for oxy and deoxy haemoglobin
    eHbO2 = Msi(haemoLUT[:, 1])
    eHbO2.set_wavelengths(haemoLUT[:, 0])
    eHb = Msi(haemoLUT[:, 2])
    eHb.set_wavelengths(haemoLUT[:, 0])
    return eHbO2, eHb


class PrahlReferenceData(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                         "haemoglobin.txt"))


class DeoxyHaemoglobinAbsorptionRaw(luigi.Task):

    def requires(self):
        return PrahlReferenceData()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            "deoxy_haemoglobin.msi"))

    def run(self):
        eHbO2, eHb = get_extinction_coefficients(self.input().path)
        writer = MsiWriter(eHb)
        writer.write(self.output().path)


class OxyHaemoglobinAbsorptionRaw(luigi.Task):

    def requires(self):
        return PrahlReferenceData()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            "oxy_haemoglobin.msi"))

    def run(self):
        eHbO2, eHb = get_extinction_coefficients(self.input().path)
        writer = MsiWriter(eHbO2)
        writer.write(self.output().path)


class SpectrometerFile(luigi.Task):
    input_file = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.FAP_IMAGE_FOLDER,
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


class PlotTransmission(luigi.Task):
    input_file = luigi.Parameter()

    def requires(self):
        return FilterTransmission(self.input_file)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
            "plot_processed_transmission" + self.input_file + ".png"))

    def run(self):
        reader = MsiReader()
        filter_transmission = reader.read(self.input().path)
        plt.figure(self.input_file)
        # to nm, because thats easier to plot
        filter_transmission.set_wavelengths(
                                filter_transmission.get_wavelengths() * 10 ** 9)
        # trow away 0 values
        filter_values = filter_transmission.get_image()
        non_zero_values = filter_values > 0
        filter_values = filter_values[non_zero_values]
        filter_transmission.set_image(filter_values)
        filter_transmission.set_wavelengths(
                filter_transmission.get_wavelengths()[non_zero_values])

        msiPlt.plot(filter_transmission)
        plt.grid()
        # create folder for figure if necessary
        head, tail = os.path.split(self.output().path)
        if not os.path.exists(head):
            os.makedirs(head)
        plt.savefig(self.output().path, dpi=500)
        plt.close(self.input_file)


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


# The full width at half maximum [m] of the used imaging systems filters
FWHM = 20 * 10 ** -9
from scipy.stats import norm
from scipy.interpolate import interp1d

def old_absorption_coefficients():


    print('create reference data...')
    # reference data

    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt("/media/wirkert/data/Data/2015_06_01_Filtertransmittance_Spectrometer/haemoglobin.txt", skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:, 0] = haemoLUT[:, 0] * 10 ** -9
    haemoLUT[:, 1:] = haemoLUT[:, 1:] * 10 ** 2
    # to account for the FWHM of the used filters, compute convolution
    # see http://en.wikipedia.org/wiki/Full_width_at_half_maximum
    filterResponse = norm(loc=0, scale=FWHM / 2.355)
    # parse at 20 locations
    # x = np.linspace(filterResponse.ppf(0.01),
    #               filterResponse.ppf(0.99), 200)

    x = np.arange(-60, 60, 2) * 10 ** -9
    filterResponse_table = filterResponse.pdf(x)
    # TODO verify if this normalization is correct!
    filterResponse_table = filterResponse_table / sum(filterResponse_table)
    haemoLUT[:, 1] = np.convolve(haemoLUT[:, 1], filterResponse_table, 'same')
    haemoLUT[:, 2] = np.convolve(haemoLUT[:, 2], filterResponse_table, 'same')

    eHbO2 = interp1d(haemoLUT[:, 0], haemoLUT[:, 1])
    eHb = interp1d(haemoLUT[:, 0], haemoLUT[:, 2])

    return eHbO2, eHb



class PlotAdaptedHaemoglobinSpectra(luigi.Task):

    def requires(self):
        return FilterTransmission("580.txt"), \
            FilterTransmission("470.txt"), \
            FilterTransmission("660.txt"), \
            FilterTransmission("560.txt"), \
            FilterTransmission("480.txt"), \
            FilterTransmission("511.txt"), \
            FilterTransmission("600.txt"), \
            FilterTransmission("700.txt"), \
            OxyHaemoglobinAbsorptionRaw(), \
            DeoxyHaemoglobinAbsorptionRaw()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
            "plot_adapted_haemoglobin.png"))

    def run(self):
        reader = MsiReader()
        # data from the original reference file
        oxy_haemoglobin = reader.read(self.input()[8].path)
        deoxy_haemoglobin = reader.read(self.input()[9].path)
        reference_wavelengths = oxy_haemoglobin.get_wavelengths()
        # these two variables will store our oxy / deoxy curves after
        # accounting filter transmissions
        oxy_haemoglobin_filter = np.zeros((8,))
        deoxy_haemoglobin_filter = np.zeros((8,))
        for i in range(len(oxy_haemoglobin_filter)):
            filter_transmission = get_transmission_data(self.input()[i].path,
                                                        reference_wavelengths)
            # build weighted sum of absorption and filter transmission
            oxy_haemoglobin_filter[i] = np.sum(oxy_haemoglobin.get_image() *
                                               filter_transmission.get_image())
            deoxy_haemoglobin_filter[i] = np.sum(deoxy_haemoglobin.get_image() *
                                               filter_transmission.get_image())
        oxy_msi_filter = Msi(oxy_haemoglobin_filter)
        oxy_msi_filter.set_wavelengths(RECORDED_WAVELENGTHS)
        deoxy_msi_filter = Msi(deoxy_haemoglobin_filter)
        deoxy_msi_filter.set_wavelengths(RECORDED_WAVELENGTHS)
        plt.figure("adapted haemolgobin")
        msiPlt.plot(oxy_msi_filter)
        msiPlt.plot(deoxy_msi_filter)
        ehbo2, ehb = old_absorption_coefficients()
        RECORDED_WAVELENGTHS_SORTED = np.array([470, 480, 511, 560, 580, 600, 660, 700]) * 10 ** -9
        plt.plot(RECORDED_WAVELENGTHS_SORTED, ehbo2(RECORDED_WAVELENGTHS_SORTED), '*')
        plt.plot(RECORDED_WAVELENGTHS_SORTED, ehb(RECORDED_WAVELENGTHS_SORTED), '*')
        plt.legend(["oxy", "deoxy", "oldoxy", "olddeoxy"])
        plt.savefig(self.output().path, dpi=500)
        plt.close("adapted haemolgobin")
        np.save("oxy.npy", oxy_msi_filter.get_image())
        np.save("deoxy.npy", deoxy_msi_filter.get_image())




if __name__ == '__main__':

    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    for root, dirs, files in os.walk(os.path.join(sp.ROOT_FOLDER,
                                                  sp.FAP_IMAGE_FOLDER)):
        for name in files:
            plot_transmission_task = PlotTransmission(input_file=name)
            w.add(plot_transmission_task)
    main_task = PlotAdaptedHaemoglobinSpectra()
    w.add(main_task)
    w.run()
