# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import Image
import ImageEnhance

import numpy as np
import luigi
import matplotlib.pyplot as plt
import SimpleITK as sitk

import tasks_analyze as at
import tasks_preprocessing as pre
import tasks_regression as rt
import scriptpaths as sp
from msi.io.pngreader import PngReader
from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.normalize as norm

sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2014_12_08_Neil_Pig_Oxygenation_Experiments/organized_recordings"
sp.DATA_FOLDER = "colon_images3"
sp.FINALS_FOLDER = "Oxy3"

def resort_wavelengths(msi):
    """Neil organized his wavelengths differently.
    This function fixes this."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [1, 5, 3, 0, 6, 7, 2, 4]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
sp.resort_wavelengths = resort_wavelengths
# the 700nm band got messed up in Neils data (way to dark). thus sort
# it out from further processing.
sp.bands_to_sortout = np.array([7])



class NeilCreateOxyImageTask(luigi.Task):
    image_prefix = luigi.Parameter()
    batch_prefix = luigi.Parameter()

    def requires(self):
        return NeilEstimateTissueParametersTask(image_prefix=self.image_prefix,
                                            batch_prefix=
                                            self.batch_prefix), \
            NeilCorrectImagingSetupTask(image_prefix=self.image_prefix), \
            NeilReprojectReflectancesTask(image_prefix=self.image_prefix,
                                      batch_prefix=
                                      self.batch_prefix)

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

        f, axarr = plt.subplots(1, 3)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [2, 5, 1]]
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
        image[image[:, :, 0] > 0.2] = 0.2  # set maximum bvf to 0.2
        at.plot_axis(axarr[1], image[:, :, 0], "blood volume fraction [%]")
        image[0, 0, 1] = 0.; image[1, 0, 1] = 1.
        at.plot_axis(axarr[2], image[:, :, 1],
                  "oxygenation [%]")

        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')


class NeilEstimateTissueParametersTask(luigi.Task):
    image_prefix = luigi.Parameter()
    batch_prefix = luigi.Parameter()

    def requires(self):
        return NeilPreprocessMSI(image_prefix=self.image_prefix), \
            rt.TrainForest(batch_prefix=
                        self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.image_prefix + "_" +
                                              self.batch_prefix +
                                              "estimate.nrrd"))

    def run(self):
        sitk_image = at.estimate_image(self.input()[0].path,
                                       self.input()[1].path)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


class NeilReprojectReflectancesTask(luigi.Task):
    image_prefix = luigi.Parameter()
    batch_prefix = luigi.Parameter()

    def requires(self):
        return NeilEstimateTissueParametersTask(image_prefix=self.image_prefix,
                                            batch_prefix=
                                            self.batch_prefix), \
            rt.TrainForestForwardModel(batch_prefix=
                        self.batch_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.image_prefix +
                                              "_backprojection_" +
                                              self.batch_prefix +
                                              "estimate.nrrd"))

    def run(self):
        sitk_image = at.estimate_image(self.input()[0].path, self.input()[1].path)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


class NeilPreprocessMSI(luigi.Task):
    image_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                  self.image_prefix + "_preprocessed.nrrd"))

    def requires(self):
        return NeilCorrectImagingSetupTask(image_prefix=self.image_prefix)

    def run(self):
        reader = NrrdReader()
        image = reader.read(self.input().path)
        image.set_image(imgmani.sortout_bands(image.get_image(),
                                              sp.bands_to_sortout))
        norm.standard_normalizer.normalize(image)
        pre.touch_and_save_msi(image, self.output())


class NeilCorrectImagingSetupTask(luigi.Task):
    """unfortunately filter were ordered differently in neils. this task is to
    do all the fiddeling to get it right again and corrects for dark image
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
        # correct for differently sorted filters if necessary
        resort_wavelengths(msi)
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
    only_F7_files = [ f for f in onlyfiles if
                 f.endswith("_F7.png") ]
    files_prefixes = [ f[:-5] for f in only_F7_files]

    for f in files_prefixes:
        main_task = NeilCreateOxyImageTask(
            image_prefix=f,
            batch_prefix=
            "jacques_no_billi_generic_scattering_")
        w.add(main_task)
    w.run()


