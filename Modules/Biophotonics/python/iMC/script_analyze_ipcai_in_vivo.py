# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import Image
import ImageEnhance
import pickle

import numpy as np
import luigi
import matplotlib.pyplot as plt
import SimpleITK as sitk
from sklearn.ensemble.forest import RandomForestRegressor
from scipy.ndimage.filters import gaussian_filter

import tasks_analyze as at
import tasks_preprocessing as pre
import tasks_mc
import scriptpaths as sp
from msi.io.pngreader import PngReader
from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.normalize as norm
from regression.estimation import estimate_image
from regression.linear import LinearSaO2Unmixing
from regression.preprocessing import preprocess



sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2014_12_08_Neil_Pig_Oxygenation_Experiments/organized_recordings"
sp.DATA_FOLDER = "colon_images_selection"
sp.FINALS_FOLDER = "Oxy_selection"

# sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

new_order = [1, 5, 3, 0, 6, 7, 2, 4]

def resort_image_wavelengths(collapsed_image):
    return collapsed_image[:, new_order]

def resort_wavelengths(msi):
    """Neil organized his wavelengths differently.
    This function fixes this."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = resort_image_wavelengths(collapsed_image)
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
sp.resort_wavelengths = resort_wavelengths

sp.bands_to_sortout = np.array([7])  # [0, 1, 2, 20, 19, 18, 17, 16])



class IPCAICreateOxyImageTask(luigi.Task):
    image_prefix = luigi.Parameter()
    batch_prefix = luigi.Parameter()

    def requires(self):
        return IPCAIEstimateTissueParametersTask(image_prefix=self.image_prefix,
                                            batch_prefix=
                                            self.batch_prefix), \
            IPCAICorrectImagingSetupTask(image_prefix=self.image_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.image_prefix + "_" +
                                              self.batch_prefix +
                                              "_summary.png"))

    def run(self):
        nrrdReader = NrrdReader()
        # read in the parametric image
        param_image = nrrdReader.read(self.input()[0].path)
        image = param_image.get_image()
        # read the multispectral image
        msi = nrrdReader.read(self.input()[1].path)

        f, axarr = plt.subplots(1, 2)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [0, 5, -1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(2.)
        plot_image = np.array(im)
        top_left_axis = axarr[0]
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.set_title("synthetic rgb",
                                fontsize=5)
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")

        # plot parametric maps
        image[np.isnan(image)] = 0.
        image[np.isinf(image)] = 0.
        image[image > 1.] = 1.
        image[image < 0.] = 0.
        image[0, 0] = 0.
        image[0, 1] = 1.
        at.plot_axis(axarr[1], image[:, :],
                  "oxygenation [%]")
        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')


class IPCAIEstimateTissueParametersTask(luigi.Task):
    image_prefix = luigi.Parameter()
    batch_prefix = luigi.Parameter()

    def requires(self):
        return IPCAIPreprocessMSI(image_prefix=self.image_prefix), \
            IPCAITrainRegressor(batch_prefix=
                        self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.image_prefix + "_" +
                                              self.batch_prefix +
                                              "estimate.nrrd"))

    def run(self):
        # load data
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        e_file = open(self.input()[1].path, 'r')
        e = pickle.load(e_file)
        # estimate
        sitk_image = estimate_image(msi, e)
        # save data
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


class IPCAITrainRegressor(luigi.Task):
    batch_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "reg_" +
                                              self.batch_prefix))

    def requires(self):
        return tasks_mc.CameraBatch(self.batch_prefix)

    def run(self):
        # extract data from the batch
        f = open(self.input().path, "r")
        batch_train = pickle.load(f)
        # TODO: fix hack by sorting always to ascending wavelengths
        batch_train.reflectances = resort_image_wavelengths(
                                                    batch_train.reflectances)
        batch_train.wavelengths = batch_train.wavelengths[new_order]
        X, y = preprocess(batch_train,
                          w_percent=0.05, bands_to_sortout=sp.bands_to_sortout)

        # train regressor
        reg = RandomForestRegressor(max_depth=30, min_samples_leaf=5, n_jobs=-1)
        reg.fit(X, y)
        # reg = LinearSaO2Unmixing()
        # save regressor
        f = self.output().open('w')
        pickle.dump(reg, f)
        f.close()


class IPCAIPreprocessMSI(luigi.Task):
    image_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                  self.image_prefix + "_preprocessed.nrrd"))

    def requires(self):
        return IPCAICorrectImagingSetupTask(image_prefix=self.image_prefix)

    def run(self):
        reader = NrrdReader()
        msi = reader.read(self.input().path)
        # sortout unwanted bands
        msi.set_image(imgmani.sortout_bands(msi.get_image(),
                                              sp.bands_to_sortout))
        # zero values would lead to infinity logarithm, thus clip.
        msi.set_image(np.clip(msi.get_image(), 0.00001, 2. ** 64))
        # normalize to get rid of lighting intensity
        norm.standard_normalizer.normalize(msi)
        # transform to absorption
        msi.set_image(-np.log(msi.get_image()))
        # normalize by l2 for stability
        norm.standard_normalizer.normalize(msi, "l2")
        # apply gaussian smoothing for error reduction
        filtered_image = gaussian_filter(msi.get_image().astype(np.float), 2)
        msi.set_image(filtered_image)
        pre.touch_and_save_msi(msi, self.output())


class IPCAICorrectImagingSetupTask(luigi.Task):
    """corrects for dark image
    and flatfield"""
    image_prefix = luigi.Parameter()

    def requires(self):
        return MultiSpectralImageFromPNGFiles(image_prefix=self.image_prefix), \
            FlatfieldFromPNGFiles()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                self.image_prefix +
                                "_image_setup_corrected.nrrd"))

    def run(self):
        """sort wavelengths and normalize by respective integration times"""
        # read inputs
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        flatfield = reader.read(self.input()[1].path)
        # correct image by flatfield
        msimani.flatfield_correction(msi, flatfield)
        pre.touch_and_save_msi(msi, self.output())


class FlatfieldFromPNGFiles(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "flatfield" +
                                ".nrrd"))

    def run(self):
        reader = PngReader()
        msi = reader.read(os.path.join(sp.ROOT_FOLDER,
                                       sp.FLAT_FOLDER,
                                       "TestImage_141211_10.19.22_"))
        writer = NrrdWriter(msi)
        seg_reader = NrrdReader()
        segmentation = seg_reader.read(os.path.join(sp.ROOT_FOLDER,
                                           sp.FLAT_FOLDER,
                                           "segmentation.nrrd"))
        msimani.apply_segmentation(msi, segmentation)
        msimani.calculate_mean_spectrum(msi)
        writer.write(self.output().path)


class MultiSpectralImageFromPNGFiles(luigi.Task):
    image_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                self.image_prefix +
                                ".nrrd"))

    def run(self):
        reader = PngReader()
        msi = reader.read(os.path.join(sp.ROOT_FOLDER,
                                       sp.DATA_FOLDER,
                                       self.image_prefix))
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


if __name__ == '__main__':

    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    image_file_folder = os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER)
    onlyfiles = [ f for f in os.listdir(image_file_folder) if
                 os.path.isfile(os.path.join(image_file_folder, f)) ]
    # each complete stack has elements F0 - F7
    only_colon_images = [ f for f in onlyfiles if
                 f.endswith("_F7.png") ]
    files_prefixes = [ f[:-5] for f in only_colon_images]

    for f in files_prefixes:
        main_task = IPCAICreateOxyImageTask(
            image_prefix=f,
            batch_prefix=
            "generic_tissue_no_aray_train")
        w.add(main_task)
    w.run()

