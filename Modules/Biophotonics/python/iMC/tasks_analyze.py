'''
Created on Aug 27, 2015

@author: wirkert
'''



import os
import numpy as np
import Image
import ImageEnhance
import pickle

import luigi
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
import SimpleITK as sitk

from msi.io.nrrdreader import NrrdReader
import msi.msimanipulations as msimani
import tasks_preprocessing as ppt
import tasks_regression as rt
import scriptpaths as sp
from regression.estimation import estimate_image


def plot_axis(axis, image, title):
    im2 = axis.imshow(image, interpolation='nearest', alpha=1.0)
    # axis.set_title(title, fontsize=5)
    # divider = make_axes_locatable(axis)
    # cax = divider.append_axes("right", size="10%", pad=0.05)
    # cbar = plt.colorbar(im2, cax=cax)
    # cbar.ax.tick_params(labelsize=5)
    axis.xaxis.set_visible(False)
    axis.yaxis.set_visible(False)

    return im2  # , cbar


class EstimateTissueParametersTask(luigi.Task):
    imageName = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return ppt.PreprocessMSI(imageName=self.imageName), \
            rt.TrainForest(batch_prefix=
                        self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.df_prefix +
                                              "estimate.nrrd"))

    def run(self):
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        # load random forest
        e_file = open(self.input()[1].path, 'r')
        e = pickle.load(e_file)

        sitk_image = estimate_image(msi, e)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


class ReprojectReflectancesTask(luigi.Task):
    imageName = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return EstimateTissueParametersTask(imageName=self.imageName,
                                            batch_prefix=
                                            self.df_prefix), \
            rt.TrainForestForwardModel(batch_prefix=
                        self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "_backprojection_" +
                                              self.df_prefix +
                                              "estimate.nrrd"))

    def run(self):
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        # load random forest
        e_file = open(self.input()[1].path, 'r')
        e = pickle.load(e_file)
        sitk_image = estimate_image(msi, e)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


def r2_evaluation(reflectances, backprojections):
    # our own version of r2_score. Cannot use the sklearn
    # one since this calculates one mean r2_score.
    # we want the r2_score for each element in the image.
    mean = np.mean(reflectances, axis=2)
    # unfortunately keepdims does not work for np.mean, thus:
    new_shape_hack = mean.shape + (1,)
    mean = np.reshape(mean, new_shape_hack)
    ss_res = np.sum(np.square(reflectances - backprojections), axis=-1)
    ss_tot = np.sum(np.square(reflectances - mean), axis=-1)

    r2_image = 1 - ss_res / ss_tot
    r2_median = np.median(r2_image)

    # santiy check: one element equals the "professional" version.
#     assert(r2_score(reflectances[0, 0, :], backprojections[0, 0, :])
#            == r2_image[0, 0])

    return r2_median, r2_image


class CreateNiceParametricImagesTask(luigi.Task):
    imageName = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return EstimateTissueParametersTask(imageName=self.imageName,
                                            batch_prefix=
                                            self.df_prefix), \
            ppt.CorrectImagingSetupTask(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName), \
            ppt.PreprocessMSI(imageName=self.imageName), \
            ReprojectReflectancesTask(imageName=self.imageName,
                                      batch_prefix=
                                      self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.df_prefix +
                                              "_summary.png"))

    def run(self):
        # read the segmentation
        reader = sitk.ImageFileReader()
        reader.SetFileName(self.input()[2].path)
        segmentation = reader.Execute()
        segmentation = sitk.GetArrayFromImage(segmentation)
        # read the multispectral image
        nrrdReader = NrrdReader()
        msi = nrrdReader.read(self.input()[1].path)
        msimani.apply_segmentation(msi, segmentation)
        # and the normalized multispectra image
        norm_msi = nrrdReader.read(self.input()[3].path)
        msimani.apply_segmentation(norm_msi, segmentation)
        # read in the parametric image
        param_image = nrrdReader.read(self.input()[0].path)
        msimani.apply_segmentation(param_image, segmentation)
        image = param_image.get_image()
        # and the reprojection
        reprojection = nrrdReader.read(self.input()[4].path)
        msimani.apply_segmentation(reprojection, segmentation)

        f, axarr = plt.subplots(2, 3)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [7, 5, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(5.)
        plot_image = np.array(im)
        top_left_axis = axarr[0, 0]
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.set_title("synthetic rgb (segmented pixels black)",
                                fontsize=5)
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")

        # plot error map
        r2_median, r2_image = r2_evaluation(norm_msi.get_image(),
                                            reprojection.get_image())
        bottom_left_axis = axarr[1, 0]
        r2_image_smaller_zero = r2_image < 0
        r2_image[r2_image_smaller_zero] = 0
        plot_axis(bottom_left_axis, r2_image, "r2 score (cut off at 0).\n" +
                     "median r2_score = " + str(r2_median))
        # plot parametric maps
        plot_axis(axarr[0, 1], image[:, :, 0], "blood volume fraction [%]")
        image[0, 0, 1] = 0.; image[1, 0, 1] = 1.
        plot_axis(axarr[0, 2], image[:, :, 1],
                  "oxygenation [%]")
        # image[0, 0, 2] = 175 * 10 ** -6; image[1, 0, 0] = 750 * 10 ** -6
        plot_axis(axarr[1, 1], image[:, :, 2] * 100,
                  "bilirubin concentration [mg/dl]")
        plot_axis(axarr[1, 2], image[:, :, 3] / 100.,
                  "mie scatter parameter [1/cm]")

        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')
