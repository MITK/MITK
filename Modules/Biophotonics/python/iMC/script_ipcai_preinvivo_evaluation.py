# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import logging
import datetime

import scipy
import pandas as pd
import numpy as np
import matplotlib.pylab as plt
import seaborn as sns
import luigi

import scriptpaths as sp
from msi.io.nrrdreader import NrrdReader
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
from msi.io.tiffringreader import TiffRingReader

"""
This file contains evaluations which are conducted before doing the real
in vivo evaluations. E.g., the determination of SNR and realistic noise levels
is done here.
"""

sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2016_01_08_Calibration_Measurements"
sp.FLAT_FOLDER = "/media/wirkert/data/Data/" + \
                 "2016_01_19_Flatfield_and_Dark_Laparoscope/Flatfield"
sp.DARK_FOLDER = "/media/wirkert/data/Data/" + \
                 "2016_01_19_Flatfield_and_Dark_Laparoscope/Dark"

# folder there the images to average lie in
AVERAGE_IMAGE_FOLDER = "/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo/" + \
                       "small_bowel_images"

LAPAROSCOPE_CARET_IMAGE_FOLDER = "/media/wirkert/data/Data/" + \
                                 "2016_01_08_Calibration_Measurements/" + \
                                 "Carets_Laparoscope"
CARET_SEGMENTATION = "/media/wirkert/data/Data/" + \
                     "2016_01_08_Calibration_Measurements/" + \
                     "caret_segmentation2.nrrd"

sns.set_style("whitegrid")


def get_image_files_from_folder(folder):
    # small helper function to get all the image files in a folder
    image_files = [f for f in os.listdir(folder) if
                   os.path.isfile(os.path.join(folder, f))]
    image_files.sort()
    image_files = [f for f in image_files if f.endswith(".tiff")]
    return image_files


class LogFit:
    """
    Small helper class to do the logarithmic fitting of SNR values
    """

    def __init__(self, x, y):
        a = np.ones((x.shape[0], 2))
        a[:, 1] = np.log(x)
        self.fit_params, res, rank, s = np.linalg.lstsq(a, y)

    def fit(self, x):
        return self.fit_params[0] + self.fit_params[1] * np.log(x)


class IPCAIAverageIntensities(luigi.Task):
    """
    collect all the average intensities calculated for each msi stack and
    store them in one dataframe
    """
    images = luigi.Parameter()
    folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "finals",
                                              "average_intensities.txt"))

    def requires(self):
        average_intensity_for_single_images = \
            [IPCAIAverageSingleImageIntensity(image=i, folder=self.folder,
                                            wavelengths=sp.RECORDED_WAVELENGTHS,
                                            segmentation=None)
             for i in self.images]
        return average_intensity_for_single_images

    def run(self):
        # first get mean intensities from all images
        df = reduce(lambda x, y: pd.concat([x, pd.read_csv(y.path, index_col=0).
                                           loc["mean"].to_frame().T]),
                    self.input()[0], pd.DataFrame())
        df.reset_index(inplace=True)

        # get the mean of all images
        df_result = df.mean(axis="rows").to_frame("mean")

        df_result.to_csv(self.output().path)


class SingleMultispectralImage(luigi.Task):
    image = luigi.Parameter()
    folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(self.folder, self.image))


class DetermineSNR(luigi.Task):
    images = luigi.Parameter()
    folder = luigi.Parameter()
    segmentation = luigi.Parameter()

    def requires(self):
        return ImageIntensityToErrorMap(images=self.images,
                                        folder=self.folder,
                                        segmentation=self.segmentation), \
               Dark(dark_folder=sp.DARK_FOLDER)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "finals",
                                              "snr.txt"))

    def run(self):
        df = pd.read_csv(self.input()[0].path, index_col=0)
        # for some reason sorting has to be done here not before, something
        # seems to be fishy with the sorting when mixed values are present.
        df.sort_values("mean", inplace=True)
        df.rename(columns={'mean': 'mean image intensity'}, inplace=True)

        # now get dark intensity
        dark = np.load(self.input()[1].path)
        # use dark to build snr
        df["SNR"] = (df["mean image intensity"]-dark) / df["std"]

        df.to_csv(self.output().path)


class DetermineSNRForBands(luigi.Task):
    images = luigi.Parameter()
    folder = luigi.Parameter()
    segmentation = luigi.Parameter()
    invivo_images = luigi.Parameter()
    invivo_folder = luigi.Parameter()

    def requires(self):
        return DetermineSNR(images=self.images,
                            folder=self.folder,
                            segmentation=self.segmentation), \
               IPCAIAverageIntensities(images=self.invivo_images,
                                       folder=self.invivo_folder)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "finals",
                                              "snr_for_bands.txt"))

    def run(self):
        df_snr = pd.read_csv(self.input()[0].path, index_col=0)
        df_invivo = pd.read_csv(self.input()[1].path, index_col=0)

        lf = LogFit(df_snr["mean image intensity"], df_snr["SNR"])
        df_invivo["SNR"] = lf.fit(df_invivo["mean"])

        df_invivo.to_csv(self.output().path)


class PlotSNR(luigi.Task):
    images = luigi.Parameter()
    folder = luigi.Parameter()
    segmentation = luigi.Parameter()
    invivo_images = luigi.Parameter()
    invivo_folder = luigi.Parameter()

    def requires(self):
        return DetermineSNR(images=self.images,
                            folder=self.folder,
                            segmentation=self.segmentation), \
               IPCAIAverageIntensities(images=self.invivo_images,
                                       folder=self.invivo_folder)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "finals",
                                              "caret_intensity_summary.pdf"))

    def run(self):
        df = pd.read_csv(self.input()[0].path, index_col=0)

        # get mean intensities for each filter as determined from invivo images
        df_invivo = pd.read_csv(self.input()[1].path, index_col=0)
        df_invivo.reset_index(inplace=True)
        df_invivo["index"] *= 10**9
        df_invivo.rename(columns={'index': 'wavelength'}, inplace=True)

        lf = LogFit(df["mean image intensity"], df["SNR"])
        df["logarithmic fit"] = lf.fit(df["mean image intensity"])

        plt.figure()
        ax = df.plot.scatter(x="mean image intensity", y="SNR")
        df.plot.line(x="mean image intensity", y="logarithmic fit", ax=ax)

        # plot the mean invivo values as rugs
        # 580nm
        plt.axvline(x=df_invivo.iloc[0, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="yellow")
        # 470nm
        plt.axvline(x=df_invivo.iloc[1, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="darkblue")
        # 660nm
        plt.axvline(x=df_invivo.iloc[2, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="red")
        # 560nm
        plt.axvline(x=df_invivo.iloc[3, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="green")
        # 480nm
        plt.axvline(x=df_invivo.iloc[4, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="blue")
        # 511nm
        plt.axvline(x=df_invivo.iloc[5, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="cyan")
        # 600nm
        plt.axvline(x=df_invivo.iloc[6, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="orange")
        # 700nm
        plt.axvline(x=df_invivo.iloc[6, 1], ymin=0, ymax=0.75,
                    linewidth=2, color="magenta")

        # annotate lowest and highest values
        y_vline = 0.75 * ax.get_ylim()[1]
        ax.annotate('470nm', xy=(df_invivo.iloc[1, 1], y_vline),
                    xycoords='data', xytext=(-37, 20), color="darkblue",
                    arrowprops=dict(facecolor='darkblue', width=1, headwidth=5),
                    textcoords='offset points')
        ax.annotate('660nm', xy=(df_invivo.iloc[2, 1], y_vline),
                    xycoords='data', xytext=(0, 20), color="red",
                    arrowprops=dict(facecolor='red', width=1, headwidth=5),
                    textcoords='offset points')
        plt.title("camera SNR and mean intensities of filter bands")

        plt.savefig(self.output().path, format="pdf")


class ImageIntensityToErrorMap(luigi.Task):
    images = luigi.Parameter()
    folder = luigi.Parameter()
    segmentation = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "finals",
                                              "caret_intensity_summary.txt"))

    def requires(self):
        average_intensity_for_single_images = \
            [IPCAIAverageSingleImageIntensity(image=i, folder=self.folder,
                                              wavelengths=[0],
                                              segmentation=self.segmentation)
             for i in self.images]
        return average_intensity_for_single_images, Dark(dark_folder=
                                                         sp.DARK_FOLDER)

    def run(self):
        # first get description information from all carets
        df = reduce(lambda x, y: pd.concat([x, pd.read_csv(y.path,
                                                           index_col=0).T]),
                    self.input()[0], pd.DataFrame())
        df = df.set_index("image name")
        df = pd.to_numeric(df, errors="coerce")
        df.to_csv(self.output().path)


class IPCAIAverageSingleImageIntensity(luigi.Task):
    image = luigi.Parameter()
    folder = luigi.Parameter()
    segmentation = luigi.Parameter()
    wavelengths = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              os.path.split(self.folder)[1],
                                              "ai_" + str(self.image) + ".txt"))

    def requires(self):
        return SingleMultispectralImage(image=self.image, folder=self.folder)

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(self.wavelengths)
        msi, segmentation = tiff_ring_reader.read(self.input().path,
                                                  nr_filters)

        # temporary writeout of tiff

        if self.segmentation is not None:
            nrrd_reader = NrrdReader()
            segmentation = nrrd_reader.read(self.segmentation)
            seg_image = segmentation.get_image()
            seg_image = scipy.misc.imresize(seg_image, 0.5, interp="bilinear",
                                            mode="F")
            msimani.apply_segmentation(msi, seg_image)

        msi.set_wavelengths(self.wavelengths)
        # calculate statistics for each band using pandas
        df = pd.DataFrame(data=imgmani.collapse_image(msi.get_image()),
                          columns=self.wavelengths)
        df = df.describe()
        # add the image name to be able to reidentify the file lateron
        df.ix["image name", 0] = self.image
        df.to_csv(self.output().path)


class Dark(luigi.Task):
    dark_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER, "dark.npy"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sp.RECORDED_WAVELENGTHS)

        # analyze all the first image files
        image_files = get_image_files_from_folder(self.dark_folder)
        image_files = filter(lambda image_name: "F0" in image_name, image_files)

        # returns the mean dark image vector of all inputted dark image
        # overly complicated TODO SW: make this simple code readable.
        dark_means = map(lambda image_name:
                            msimani.calculate_mean_spectrum(
                                tiff_ring_reader.read(os.path.join(self.dark_folder,
                                                                   image_name),
                                                      nr_filters)[0]),
                         image_files)
        dark_means_sum = reduce(lambda x, y: x+y.get_image(), dark_means, 0)
        final_dark_mean = dark_means_sum / len(dark_means)
        # dark should be the same for all wavebands
        final_dark_mean = np.mean(final_dark_mean)

        np.save(self.output().path, final_dark_mean)


if __name__ == '__main__':

    # root folder there the data lies
    logging.basicConfig(filename='small_bowel' + str(datetime.datetime.now()) +
                        '.log', level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # do the average image calculation for all small bowel images
    average_invivo_image_files = get_image_files_from_folder(
            AVERAGE_IMAGE_FOLDER)
    # a task to calucualte the average intensity for all images
    # always start with the filter 0 to have consistent order on the bands
    first_invivo_image_files = filter(lambda image_name: "F0" in image_name,
                                      average_invivo_image_files)

    # plot snr for image intensity variation using lots of images at
    # different intensities c.f. a color checker board.
    intensity_patch_files = get_image_files_from_folder(
            LAPAROSCOPE_CARET_IMAGE_FOLDER)
    plot_snr_task = PlotSNR(
            images=intensity_patch_files,
            folder=LAPAROSCOPE_CARET_IMAGE_FOLDER,
            segmentation=CARET_SEGMENTATION,
            invivo_images=first_invivo_image_files,
            invivo_folder=AVERAGE_IMAGE_FOLDER)
    w.add(plot_snr_task)
    # the results of this task will be used by the in-vivo evaluation to
    # set the snr of specific bands to the correct value
    snr_for_bands_task = DetermineSNRForBands(
            images=intensity_patch_files,
            folder=LAPAROSCOPE_CARET_IMAGE_FOLDER,
            segmentation=CARET_SEGMENTATION,
            invivo_images=first_invivo_image_files,
            invivo_folder=AVERAGE_IMAGE_FOLDER)
    w.add(snr_for_bands_task)

    # run both tasks
    w.run()

