# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import numpy as np
import Image
import ImageEnhance

import matplotlib.pyplot as plt
import luigi
import SimpleITK as sitk

from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
from msi.io.spectrometerreader import SpectrometerReader
import msi.plot as msiPlt
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
from msi.normalize import NormalizeMean
import scriptpaths as sp
import tasks_preprocessing as ppt


# root folder there the data lies
sp.ROOT_FOLDER = "/media/wirkert/data/Data/2015_08_31_new_colorcheckertest/"
sp.DATA_FOLDER = "colors"
RECORDED_WAVELENGTHS = \
        np.array([580, 470, 660, 560, 480, 511, 600, 700]) * 10 ** -9


def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
ppt.resort_wavelengths = resort_wavelengths



class SegmentationFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 "segmentation.nrrd"))


class SpectrometerFile(luigi.Task):
    image_name = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.DATA_FOLDER,
                                         "color_Reflection_000" +
                                         os.path.splitext(self.image_name)[0] +
                                         ".txt"))


class GetDataFromSpectrometerTask(luigi.Task):
    image_name = luigi.Parameter()

    def requires(self):
        return SpectrometerFile(self.image_name)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
            "ColorChecker_" + self.image_name + "_spectrometer.nrrd"))

    def run(self):
        reader = SpectrometerReader()
        image = reader.read(self.input().path)
        msimani.interpolate_wavelengths(image, RECORDED_WAVELENGTHS)
        # create folder for figure if necessary
        head, tail = os.path.split(self.output().path)
        if not os.path.exists(head):
            os.makedirs(head)
        writer = NrrdWriter(image)
        writer.write(self.output().path)



class GetDataFromSpectrocamTask(luigi.Task):
    image_name = luigi.Parameter()

    def requires(self):
        return ppt.CorrectImagingSetupTask(self.image_name), \
            SegmentationFile()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
             "ColorChecker_" + self.image_name + "_spectrocam.nrrd"))

    def run (self):
        reader = NrrdReader()
        image = reader.read(self.input()[0].path)
        image.set_wavelengths(RECORDED_WAVELENGTHS)
        sitk_reader = sitk.ImageFileReader()
        sitk_reader.SetFileName(self.input()[1].path)
        segmentation_image = sitk_reader.Execute()
        segmentation_image = sitk.GetArrayFromImage(segmentation_image)
        msimani.apply_segmentation(image, segmentation_image)
        msimani.calculate_mean_spectrum(image)

        # create folder for figure if necessary
        head, tail = os.path.split(self.output().path)
        if not os.path.exists(head):
            os.makedirs(head)
        writer = NrrdWriter(image)
        writer.write(self.output().path)


class PlotMeasuredSpectraForImage(luigi.Task):
    image_name = luigi.Parameter()

    def requires(self):
        return GetDataFromSpectrocamTask(self.image_name), \
            GetDataFromSpectrometerTask(self.image_name), \
            ppt.CorrectImagingSetupTask(self.image_name)


    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
             "ColorChecker_" + self.image_name + "_summary.png"))

    def run(self):
        # get data
        reader = NrrdReader()
        data_spectrocam = reader.read(self.input()[0].path)
        data_spectrocam.set_wavelengths(RECORDED_WAVELENGTHS)
        data_spectrometer = reader.read(self.input()[1].path)
        data_spectrometer.set_wavelengths(RECORDED_WAVELENGTHS)
        msi = reader.read(self.input()[2].path)
        # make to subplots
        f, axarr = plt.subplots(1, 2)

        # first plot: the synthetic rgb
        rgb_image = msi.get_image()[:, :, [7, 5, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(4.)
        plot_image = np.array(im)
        left_axes = axarr[0]
        left_axes.imshow(plot_image, interpolation='nearest')
        left_axes.set_title("synthetic rgb",
                                fontsize=5)
        left_axes.xaxis.set_visible(False)
        left_axes.yaxis.set_visible(False)

        # second plot: the normalized spectra
        normalizer = NormalizeMean()
        normalizer.normalize(data_spectrocam)
        normalizer.normalize(data_spectrometer)
        right_axes = axarr[1]
        right_axes.grid()
        msiPlt.plot(data_spectrocam, right_axes)
        msiPlt.plot(data_spectrometer, right_axes)
        right_axes.legend(["spectrocam", "spectrometer"],
                          bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
                          ncol=2, mode="expand", borderaxespad=0.,
                          fontsize=5)

        # create folder for figure if necessary
        head, tail = os.path.split(self.output().path)
        if not os.path.exists(head):
            os.makedirs(head)
        plt.savefig(self.output().path, dpi=500)
        plt.close(self.image_name)

if __name__ == '__main__':


    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    for root, dirs, files in os.walk(os.path.join(sp.ROOT_FOLDER,
                                                  sp.DATA_FOLDER)):
        for name in files:
            if name.endswith(".nrrd"):
                color_checking = PlotMeasuredSpectraForImage(image_name=name)
                w.add(color_checking)
    w.run()


