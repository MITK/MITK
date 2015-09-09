'''
Created on Aug 27, 2015

@author: wirkert
'''



import Image
import ImageEnhance
import os
import numpy as np
import copy
import logging

import luigi
from sklearn.decomposition import FastICA
from sklearn.decomposition import PCA
import SimpleITK as sitk
import matplotlib.pyplot as plt

import msi.msimanipulations as msimani
import preproctasks as ppt
import analyzetasks as at
import scriptpaths as sp
from msi.io.nrrdreader import NrrdReader
from msi.imgmani import select_n_reflectances
import msi.imgmani as imgmani

# folder there the bss specific output shall be written to
sp.FINALS_FOLDER = "bss"
# number of components we seperate our signal in:
NR_COMPONENTS = 4
# randomly draw this many samples from image for bss
NR_SAMPLES = 100000

def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recording the wavelengths got messed up
ppt.resort_wavelengths = resort_wavelengths


def plot_axis(axis, image, title):
    im, cbar = at.plot_axis(axis, image, title)
    cbar.ax.set_yticklabels([])


def clamp_img_percentiles(image, p_low, p_high):
    p_low_value = np.percentile(image, p_low)
    low_elems = image < p_low_value
    image[low_elems] = p_low_value

    p_high_value = np.percentile(image, p_high)
    high_elems = image > p_high_value
    image[high_elems] = p_high_value


def preprocess(img_path, segmentation_path):
    """ get data ready to be processed by bss methods (ica, pca) """
    # read the segmentation
    reader = sitk.ImageFileReader()
    reader.SetFileName(segmentation_path)
    segmentation = reader.Execute()
    segmentation = sitk.GetArrayFromImage(segmentation)
    # read the image
    nrrdReader = NrrdReader()
    img = nrrdReader.read(img_path)
    msimani.apply_segmentation(img, segmentation)

    img_copy = copy.deepcopy(img)
    bss_input = imgmani.remove_masked_elements(
                                                img_copy.get_image())
    select_n_reflectances(bss_input, NR_SAMPLES)

    return bss_input, img


class CalculateIcaTask(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return ppt.PreprocessMSI(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName)


    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "_ica_sources" +
                                              ".nrrd"))
    def run(self):
        bss_input, img = preprocess(self.input()[0].path, self.input()[1].path)

        ica = FastICA(NR_COMPONENTS)
        ica.fit(bss_input)
        transformed_image = ica.transform(img.get_image())

        # save as sitk nrrd.
        sitk_img = sitk.GetImageFromArray(transformed_image,
                                 isVector=True)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_img, self.output().path)
        logging.info(ica)


class CalculatePcaTask(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return ppt.PreprocessMSI(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName)


    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "_pca_sources" +
                                              ".nrrd"))
    def run(self):
        bss_input, img = preprocess(self.input()[0].path, self.input()[1].path)

        pca = PCA(NR_COMPONENTS)
        pca.fit(bss_input)
        transformed_image = pca.transform(img.get_image())

        # save as sitk nrrd.
        sitk_img = sitk.GetImageFromArray(transformed_image,
                                 isVector=True)
        outFile = self.output().open('w')
        outFile.close()
        sitk.WriteImage(sitk_img, self.output().path)
        logging.info(pca)


class PlotBssImagesTask(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return ppt.CorrectImagingSetupTask(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName), \
            CalculateIcaTask(imageName=self.imageName), \
            CalculatePcaTask(imageName=self.imageName)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName +
                                              "_bss_sources_" +
                                              "summary.png"))

    def run(self):
        # read the segmentation
        reader = sitk.ImageFileReader()
        reader.SetFileName(self.input()[1].path)
        segmentation = reader.Execute()
        segmentation = sitk.GetArrayFromImage(segmentation)
        # read the multispectral image
        nrrdReader = NrrdReader()
        msi = nrrdReader.read(self.input()[0].path)
        msimani.apply_segmentation(msi, segmentation)
        # read the ica components
        ica_image = nrrdReader.read(self.input()[2].path)
        msimani.apply_segmentation(ica_image, segmentation)
        ica_image = ica_image.get_image()
        # read the pca components
        pca_image = nrrdReader.read(self.input()[3].path)
        msimani.apply_segmentation(pca_image, segmentation)
        pca_image = pca_image.get_image()

        f, axarr = plt.subplots(3, ica_image.shape[-1])

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [7, 5, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(7.)
        plot_image = np.array(im)

        top_left_axis = axarr[0, 0]
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.set_title("synthetic rgb",
                                fontsize=5)
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")

        # plot parametric maps
        for i in range(ica_image.shape[-1]):
            # upper row axis off (no images)
            axarr[0, i].axis('off')
            # plot ica
            clamp_img_percentiles(ica_image[:, :, i], 5, 95)
            plot_axis(axarr[1, i], ica_image[:, :, i], "ica C" + str(i))
            # plot pca
            clamp_img_percentiles(pca_image[:, :, i], 5, 95)
            plot_axis(axarr[2, i], pca_image[:, :, i], "pca C" + str(i))

        plt.savefig(self.output().path , dpi=1000, bbox_inches='tight')



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
            main_task = PlotBssImagesTask(imageName=name)
            w.add(main_task)
    w.run()
