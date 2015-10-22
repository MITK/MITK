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
from sklearn.preprocessing import Normalizer
from sklearn.linear_model import LinearRegression

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


sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_in_vivo"
sp.DATA_FOLDER = "colon_images"
sp.FINALS_FOLDER = "Oxy"
sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

sp.bands_to_sortout = np.array([])  # [0, 1, 2, 20, 19, 18, 17, 16])



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
        # e = LinearSaO2Unmixing()
        sitk_image = estimate_image(msi, e)
        # save data
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


def extract_batch_data(filename):
    # get data
    f = open(filename, "r")
    batch = pickle.load(f)
    reflectances = batch.reflectances
    y = batch.layers[0][:, 1]
    # add noise to reflectances
    noises = np.random.normal(loc=0., scale=0.3, size=reflectances.shape)
    reflectances += noises * reflectances
    reflectances = np.clip(reflectances, 0.00001, 1.)
    # normalize reflectances
    normalizer = Normalizer(norm='l1')
    reflectances = normalizer.transform(reflectances)
    # reflectances to absorption
    absorptions = -np.log(reflectances)
    X = absorptions
    # get rid of sorted out bands
    X = np.delete(X, sp.bands_to_sortout, axis=1)
    return X, y


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
        X, y = extract_batch_data(self.input().path)
        # train regressor
        linear_regressor = LinearRegression()
        linear_regressor.fit(X, y)
        # save regressor
        f = self.output().open('w')
        pickle.dump(linear_regressor, f)
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
        image = reader.read(self.input().path)
        image.set_image(imgmani.sortout_bands(image.get_image(),
                                              sp.bands_to_sortout))
        norm.standard_normalizer.normalize(image)
        image.set_image(-np.log(image.get_image()))
        pre.touch_and_save_msi(image, self.output())


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
                                       "image sample "))
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
                 f.endswith("670nm.png") ]
    files_prefixes = [ f[:-9] for f in only_colon_images]

    for f in files_prefixes:
        main_task = IPCAICreateOxyImageTask(
            image_prefix=f,
            batch_prefix=
            "generic_tissue")
        w.add(main_task)
    w.run()

