# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import Image
import ImageEnhance
import logging
import datetime
import copy

from scipy.interpolate import interp1d
from sklearn.preprocessing import normalize
from sklearn.metrics import r2_score

from msi.io.nrrdreader import NrrdReader
import msi.normalize as norm
from ipcai2016.tasks_common import *
import commons
from ipcai2016 import tasks_mc
import msi.plot as msiplot
from msi.io.spectrometerreader import SpectrometerReader
from msi.normalize import standard_normalizer
from msi.io.tiffringreader import TiffRingReader

TiffRingReader.RESIZE_FACTOR = 0.5

sc = commons.ScriptCommons()

sc.add_dir("COLORCHECKER_DATA",
           os.path.join(sc.get_dir("DATA_FOLDER"), "colorchecker_laparoscope"))

sc.add_dir("COLORCHECKER_RESULTS",
           os.path.join(sc.get_dir("RESULTS_FOLDER"),
                        "colorchecker_laparoscope"))

sc.add_dir("FLAT_FOLDER",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "colorchecker_laparoscope_flatfield"))

sc.add_dir("SPECTROMETER_REFERENCE_DATA",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "spectrometer_reflectance_new"))

sc.add_dir("FILTER_TRANSMISSIONS",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "filter_transmissions"))


class CheckColorCheckerBoards(luigi.Task):
    image_name = luigi.Parameter()

    def requires(self):
        return Flatfield(flatfield_folder=sc.get_full_dir("FLAT_FOLDER")), \
               SingleMultispectralImage(image=self.image_name), \
               Dark(dark_folder=sc.get_full_dir("DARK_FOLDER")), \
               SpectrometerToSpectrocam(spectrometer_measurement=
                                        os.path.join(sc.get_full_dir("SPECTROMETER_REFERENCE_DATA"),
                                                     os.path.split(self.image_name)[1][0:8] + ".txt"))

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("COLORCHECKER_RESULTS"),
                                              os.path.split(self.image_name)[1] +
                                              "_" + "_color_check" + ".png"))

    def run(self):

        print "... read data"

        segmentation_file = os.path.join(
                sc.get_full_dir("COLORCHECKER_DATA"), "seg.tiff")
        segmentation_file2 = os.path.join(
                sc.get_full_dir("COLORCHECKER_DATA"), "seg2.tiff")

        nrrd_reader = NrrdReader()
        tiff_ring_reader = TiffRingReader()
        # read the flatfield
        flat = nrrd_reader.read(self.input()[0].path)
        dark = nrrd_reader.read(self.input()[2].path)
        # read the msi
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])
        msi, segmentation = tiff_ring_reader.read(self.input()[1].path,
                                                  nr_filters,
                                                  segmentation=segmentation_file)
        msi_copy = copy.deepcopy(msi)  # copy to be able to apply both
        # segmentations
        msimani.apply_segmentation(msi, segmentation)
        msi2, segmentation2 = tiff_ring_reader.read(self.input()[1].path,
                                                    nr_filters,
                                                    segmentation=segmentation_file2)
        msimani.apply_segmentation(msi2, segmentation2)

        msimani.apply_segmentation(msi_copy, segmentation + segmentation2)

        # read the spectrometer measurement
        msi_spectrometer = nrrd_reader.read(self.input()[3].path)

        # correct by flatfield and dark image
        #msimani.image_correction(msi, flat, dark)
        #msimani.image_correction(msi2, flat, dark)
        #msimani.image_correction(msi_copy, flat, dark)
        msimani.dark_correction(msi, dark)
        msimani.dark_correction(msi2, dark)
        msimani.dark_correction(msi_copy, dark)

        # create artificial rgb
        rgb_image = msi_copy.get_image()[:, :, [2, 3, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.

        # preprocess the image
        # sortout unwanted bands
        print "... apply normalizations"
        # normalize to get rid of lighting intensity
        norm.standard_normalizer.normalize(msi)
        norm.standard_normalizer.normalize(msi2)

        print "... plot"

        # plot of the rgb image
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(2.)
        plotted_image = np.array(im)

        plt.figure()
        f, (ax_rgb, ax_spectra) = plt.subplots(1, 2)
        plot_image(plotted_image, ax_rgb, title="false rgb")

        msiplot.plotMeanError(msi, ax_spectra)
        msiplot.plotMeanError(msi2, ax_spectra)

        standard_normalizer.normalize(msi_spectrometer)
        msiplot.plot(msi_spectrometer, ax_spectra)

        mean_spectrometer = msi_spectrometer.get_image()
        mean_msi = msimani.calculate_mean_spectrum(msi).get_image()
        mean_msi2 = msimani.calculate_mean_spectrum(msi2).get_image()
        r2_msi = r2_score(mean_spectrometer, mean_msi)
        r2_msi2 = r2_score(mean_spectrometer, mean_msi2)

        ax_spectra.legend(["spectrocam 1 r2: " + str(r2_msi),
                           "spectrocam 2 r2: " + str(r2_msi2), "spectrometer"],
                  bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
                  ncol=2, mode="expand", borderaxespad=0.,
                  fontsize=5)

        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight')


class CameraQEFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("DATA_FOLDER"),
                                              "camera_quantum_efficiency.csv"))


class SpectrometerToSpectrocam(luigi.Task):

    spectrometer_measurement = luigi.Parameter()

    def requires(self):
        # all wavelengths must have been measured for transmission and stored in
        # wavelength.txt files (e.g. 470.txt)
        filenames = ((sc.other["RECORDED_WAVELENGTHS"] * 10**9).astype(int)).astype(str)
        filenames = map(lambda name: tasks_mc.FilterTransmission(os.path.join(sc.get_full_dir("FILTER_TRANSMISSIONS"),
                                                                              name) + ".txt"),
                        filenames)

        return tasks_mc.SpectrometerFile(self.spectrometer_measurement), \
               filenames, CameraQEFile()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("INTERMEDIATES_FOLDER"),
                                              os.path.split(self.spectrometer_measurement)[1] +
                                              "_spectrocam.nrrd"))

    def run(self):
        # load spectrometer measurement
        spectrometer_reader = SpectrometerReader()
        spectrometer_msi = spectrometer_reader.read(self.input()[0].path)

        # the wavelengths recorded by the spectrometer
        spectrometer_wavelengths = spectrometer_msi.get_wavelengths()

        spectrometer_white = spectrometer_reader.read(os.path.join(
                sc.get_full_dir("DATA_FOLDER"), "spectrometer_whitebalance",
                "white_IL_1_OO_20ms.txt"))
        spectrometer_dark = spectrometer_reader.read(os.path.join(
                sc.get_full_dir("DATA_FOLDER"), "spectrometer_whitebalance",
                "dark_1_OO_20ms.txt"))
        msimani.dark_correction(spectrometer_white, spectrometer_dark)
        white_interpolator = interp1d(spectrometer_white.get_wavelengths(),
                                      spectrometer_white.get_image(),
                                      bounds_error=False, fill_value=0.)
        white_interpolated = white_interpolator(spectrometer_wavelengths)

        camera_qe = pd.read_csv(self.input()[2].path)
        camera_qe_interpolator = interp1d(camera_qe["wavelengths"] * 10**-9,
                                          camera_qe["quantum efficiency"],
                                          bounds_error=False,
                                          fill_value=0.)
        camera_qe_interpolated = \
            camera_qe_interpolator(spectrometer_wavelengths)

        # camera batch creation:
        new_reflectances = []
        for band in self.input()[1]:
            df_filter = pd.read_csv(band.path)
            interpolator = interp1d(df_filter["wavelengths"],
                                    df_filter["reflectances"],
                                    assume_sorted=False, bounds_error=False)
            # use this to create new reflectances
            interpolated_filter = interpolator(spectrometer_wavelengths)
            # if a wavelength cannot be interpolated, set it to 0
            interpolated_filter = np.nan_to_num(interpolated_filter)
            # account for cameras quantum efficiency
            interpolated_filter *= camera_qe_interpolated * white_interpolated
            # normalize band response
            #normalize(interpolated_filter.reshape(1, -1), norm='l1', copy=False)
            folded_reflectance = np.dot(spectrometer_msi.get_image(),
                                        interpolated_filter)
            new_reflectances.append(folded_reflectance)
            plt.plot(interpolated_filter)
        new_reflectances = np.array(new_reflectances).T
        spectrometer_msi.set_image(new_reflectances,
                                   wavelengths=sc.other["RECORDED_WAVELENGTHS"])

        # write it
        nrrd_writer = NrrdWriter(spectrometer_msi)
        nrrd_writer.write(self.output().path)

if __name__ == '__main__':

    # create a folder for the results if necessary
    sc.set_root("/media/wirkert/data/Data/2020_Current_Works/")
    sc.create_folders()

    # root folder there the data lies
    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                 "color_checker" +
                                 str(datetime.datetime.now()) +
                                 '.log'), level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    files = get_image_files_from_folder(sc.get_full_dir("COLORCHECKER_DATA"),
                                        suffix="F0.tiff", fullpath=True)

    for f in files:
        main_task = CheckColorCheckerBoards(image_name=f)
        w.add(main_task)

    w.run()

