# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import numpy as np
import pickle
import Image
import ImageEnhance

import luigi
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
import SimpleITK as sitk

from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
from msi.normalize import NormalizeIQ
import msi.imgmani as imgmani

ROOT_FOLDER = "/media/wirkert/data/Data/2015_05_28_ColonUCCancerAndPolyps"
FLAT_FOLDER = "flatfields"
DARK_FOLDER = "dark"
FAP_IMAGE_FOLDER = "colon_images"




def smooth(msi):
    """ helper function to gaussian smooth msi channel by channel. """
    img = sitk.GetImageFromArray(msi.get_image(), isVector=True)
    smoothFilter = sitk.SmoothingRecursiveGaussianImageFilter()
    smoothFilter.SetSigma(4)
    img_smoothed = smoothFilter.Execute(img)
    img_array = sitk.GetArrayFromImage(img_smoothed)
    msi.set_image(img_array)


def touch_and_save_msi(msi, outfile):
    """ saves msi as a nrrd to outfile.
    if the directory / file does not exist it will be created """
    # touch file so path definately exists
    _outFile = outfile.open('w')
    _outFile.close()
    # use nrrd writer to write file
    _out = outfile
    writer = NrrdWriter(msi)
    writer.write(_out.path)


def plot_axis(axis, image, title):
    im2 = axis.imshow(image, interpolation='nearest')
    axis.set_title(title, fontsize=5)
    divider_dsp = make_axes_locatable(axis)
    cax_dsp = divider_dsp.append_axes("right", size="10%", pad=0.05)
    cbar = plt.colorbar(im2, cax=cax_dsp)
    cbar.ax.tick_params(labelsize=5)
    axis.xaxis.set_visible(False)
    axis.yaxis.set_visible(False)


class MultiSpectralImageFile(luigi.Task):
    imageName = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, FAP_IMAGE_FOLDER,
                                self.imageName))


class CorrectImagingSetupTask(luigi.Task):
    """unfortunately filter were ordered weirdly. this task is to
    do all the fiddeling to get it right again"""
    imageName = luigi.Parameter()

    def requires(self):
        return MultiSpectralImageFile(imageName=self.imageName), \
            PreprocessFlatfields(), \
            PreprocessDark()


    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                self.imageName +
                                "_image_setup_corrected.nrrd"))

    def run(self):
        """sort wavelengths and normalize by respective integration times"""
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        if "long" in self.input()[0].path:
            msi.add_property({'integration times':
                np.array([150., 250., 117., 160., 150., 175., 82., 70.])})
        else:
            msi.add_property(
                    {'integration times':
                     np.array([30., 50., 47., 32., 30., 35., 35., 60.])})
        flatfield = reader.read(self.input()[1].path)
        flatfield.add_property({'integration times':
                     np.array([30., 50., 47., 32., 30., 35., 35., 60.])})
        dark = reader.read(self.input()[2].path)
        msimani.image_correction(msi, flatfield, dark)
        touch_and_save_msi(msi, self.output())


class PreprocessFlatfields(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                  "flatfield.nrrd"))

    def requires(self):
        return luigi.Task.requires(self)

    def run(self):
        reader = NrrdReader()
        flatfield_folder = os.path.join(ROOT_FOLDER, FLAT_FOLDER)
        flatfield_nrrds = os.listdir(flatfield_folder)
        #  map to full file path
        flatfield_nrrds = \
            [os.path.join(flatfield_folder, f) for f in flatfield_nrrds]

        seg_string = ""
        img_string = ""
        if "flatSegmentation" in flatfield_nrrds[0]:
            seg_string = flatfield_nrrds[0]
            img_string = flatfield_nrrds[1]
        else:
            seg_string = flatfield_nrrds[1]
            img_string = flatfield_nrrds[0]

        flatfield = reader.read(img_string)
        sitk_reader = sitk.ImageFileReader()
        sitk_reader.SetFileName(seg_string)
        flatfield_segmentation = sitk_reader.Execute()
        flatfield_segmentation = sitk.GetArrayFromImage(flatfield_segmentation)

        msimani.apply_segmentation(flatfield, flatfield_segmentation)
        msimani.calculate_mean_spectrum(flatfield)
        touch_and_save_msi(flatfield, self.output())


class PreprocessDark(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                  "dark.nrrd"))

    def requires(self):
        return luigi.Task.requires(self)

    def run(self):
        reader = NrrdReader()
        dark_folder = os.path.join(ROOT_FOLDER, DARK_FOLDER)
        dark_nrrds = os.listdir(dark_folder)
        #  map to full file path
        dark_nrrds = [os.path.join(dark_folder, d) for d in dark_nrrds]
        dark = reader.read(dark_nrrds[0])  # just take the first dark image
        # alternatively multiple dark images could be averaged to one.

        # apply sitk gaussian smoothing to dark result
        smooth(dark)
        touch_and_save_msi(dark, self.output())


class PreprocessMSI(luigi.Task):
    imageName = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                  self.imageName + "_preprocessed.nrrd"))

    def requires(self):
        return CorrectImagingSetupTask(imageName=self.imageName)


    def run(self):
        reader = NrrdReader()
        image = reader.read(self.input().path)
        # for this special recording, the sorting of the wavelengths
        # got messed up.this fixes that.
        normalizer = NormalizeIQ()
        normalizer.normalize(image)
        touch_and_save_msi(image, self.output())


class RegressorFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER,
                                 "rf"))


class EstimateTissueParametersTask(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return PreprocessMSI(imageName=self.imageName), \
            RegressorFile()

    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "estimate.nrrd"))

    def run(self):
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        # load random forest
        rf_file = self.input()[1].open('r')
        rf = pickle.load(rf_file)
        # estimate parameters
        collapsed_msi = imgmani.collapse_image(msi.get_image())
        estimated_parameters = rf.predict(collapsed_msi)
        # restore shape
        estimated_paramters_as_image = np.reshape(
                estimated_parameters, (msi.get_image().shape[0],
                                       msi.get_image().shape[1],
                                       estimated_parameters.shape[1]))
        # save as sitk nrrd.
        img = sitk.GetImageFromArray(estimated_paramters_as_image,
                                     isVector=True)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(img, self.output().path)


class CreateNiceParametricImagesTask(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return EstimateTissueParametersTask(imageName=self.imageName), \
            CorrectImagingSetupTask(imageName=self.imageName)


    def output(self):
        return luigi.LocalTarget(os.path.join(ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName + "summary.png"))

    def run(self):
        # read in the parametric image
        reader = sitk.ImageFileReader()
        reader.SetFileName(self.input()[0].path)
        image = reader.Execute()
        image = sitk.GetArrayFromImage(image)
        # read the multispectral image
        msireader = NrrdReader()
        msi = msireader.read(self.input()[1].path)

        f, axarr = plt.subplots(1, 5)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [2, 5, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(3.)

        top_left_axis = axarr[0]
        top_left_axis.imshow(np.array(im), interpolation='nearest')
        top_left_axis.set_title("synthetic rgb", fontsize=5)
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        # plot parametric maps
        plt.set_cmap("jet")
        plot_axis(axarr[1], image[:, :, 0], "blood volume fraction [%]")
        plot_axis(axarr[2], image[:, :, 1],
                  "density scattering particles [%]")
        plot_axis(axarr[3], image[:, :, 2] * 10 ** 6,
                  "mucosa thickness [um]")
        plot_axis(axarr[4], image[:, :, 3], "oxygenation [%]")

        plt.savefig(self.output().path, dpi=500, bbox_inches='tight')





if __name__ == '__main__':

    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    for root, dirs, files in os.walk(os.path.join(ROOT_FOLDER,
                                                  FAP_IMAGE_FOLDER)):
        for name in files:
            main_task = CreateNiceParametricImagesTask(
                imageName=name)
            w.add(main_task)
    w.run()


