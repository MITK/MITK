'''
Created on Aug 27, 2015

@author: wirkert
'''



import os
import numpy as np
import pickle
import Image
import ImageEnhance

import luigi
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
from sklearn.metrics import r2_score
import SimpleITK as sitk

from msi.io.nrrdreader import NrrdReader
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import preproctasks as ppt
import regressiontasks as rt
import scriptpaths as sp


def plot_axis(axis, image, title):
    im2 = axis.imshow(image, interpolation='nearest', alpha=1.0)
    axis.set_title(title, fontsize=5)
    divider_dsp = make_axes_locatable(axis)
    cax_dsp = divider_dsp.append_axes("right", size="10%", pad=0.05)
    cbar = plt.colorbar(im2, cax=cax_dsp)
    cbar.ax.tick_params(labelsize=5)
    axis.xaxis.set_visible(False)
    axis.yaxis.set_visible(False)

    return im2, cbar


def estimate_image(image_filename, rf_filename):
    reader = NrrdReader()
    msi = reader.read(image_filename)
    # load random forest
    rf_file = open(rf_filename, 'r')
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
    sitk_img = sitk.GetImageFromArray(estimated_paramters_as_image,
                                 isVector=True)
    return sitk_img


class EstimateTissueParametersTask(luigi.Task):
    imageName = luigi.Parameter()
    file_name_prefix_training = luigi.Parameter()

    def requires(self):
        return ppt.PreprocessMSI(imageName=self.imageName), \
            rt.TrainForest(file_name_prefix_training=
                        self.file_name_prefix_training)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.file_name_prefix_training +
                                              "estimate.nrrd"))

    def run(self):
        sitk_image = estimate_image(self.input()[0].path, self.input()[1].path)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_image, self.output().path)


class ReprojectReflectancesTask(luigi.Task):
    imageName = luigi.Parameter()
    file_name_prefix_training = luigi.Parameter()

    def requires(self):
        return EstimateTissueParametersTask(imageName=self.imageName,
                                            file_name_prefix_training=
                                            self.file_name_prefix_training), \
            rt.TrainForestForwardModel(file_name_prefix_training=
                        self.file_name_prefix_training)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "_backprojection_" +
                                              self.file_name_prefix_training +
                                              "estimate.nrrd"))

    def run(self):
        sitk_image = estimate_image(self.input()[0].path, self.input()[1].path)
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
    file_name_prefix_training = luigi.Parameter()

    def requires(self):
        return EstimateTissueParametersTask(imageName=self.imageName,
                                            file_name_prefix_training=
                                            self.file_name_prefix_training), \
            ppt.CorrectImagingSetupTask(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName), \
            ppt.PreprocessMSI(imageName=self.imageName), \
            ReprojectReflectancesTask(imageName=self.imageName,
                                      file_name_prefix_training=
                                      self.file_name_prefix_training)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.file_name_prefix_training +
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
        top_left_axis.set_title("synthetic rgb (segmented pixels balck)",
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
        image[0, 0, 0] = 0.; image[1, 0, 0] = 0.2
        plot_axis(axarr[0, 1], image[:, :, 0], "blood volume fraction [%]")
        image[0, 0, 1] = 0.; image[1, 0, 1] = 0.6
        plot_axis(axarr[0, 2], image[:, :, 1], "density scattering particles [%]")
        image[0, 0, 2] = 175 * 10 ** -6; image[1, 0, 0] = 750 * 10 ** -6
        plot_axis(axarr[1, 1], image[:, :, 2] * 10 ** 6, "mucosa thickness [um]")
        image[0, 0, 3] = 0.; image[1, 0, 3] = 1.
        plot_axis(axarr[1, 2], image[:, :, 3], "oxygenation [%]")

        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')
