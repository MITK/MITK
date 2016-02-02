# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import Image
import ImageEnhance
import pickle
import logging
import datetime

import numpy as np
import pandas as pd
import luigi
import matplotlib.pyplot as plt
import SimpleITK as sitk
from sklearn.ensemble.forest import RandomForestRegressor
import seaborn as sns

import tasks_analyze as at
import tasks_mc
import scriptpaths as sp
from msi.msi import Msi
from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.normalize as norm
from regression.estimation import estimate_image
from regression.preprocessing import preprocess, preprocess2
from msi.io.tiffringreader import TiffRingReader


sp.ROOT_FOLDER = "/media/wirkert/data/Data/2015_11_12_IPCAI_in_vivo"
sp.DATA_FOLDER = "all_liver_images"
sp.FINALS_FOLDER = "liver_all_SNR10"
sp.FLAT_FOLDER = "/media/wirkert/data/Data/" + \
                 "2016_01_19_Flatfield_and_Dark_Laparoscope/Flatfields"
sp.DARK_FOLDER = "/media/wirkert/data/Data/" + \
                 "2016_01_19_Flatfield_and_Dark_Laparoscope/Dark"

AVERAGE_FOLDER = "average_intensity"
AVERAGE_FINALS_FOLDER = "finals"

sns.set_style("whitegrid")

font = {'family' : 'normal',
        'size'   : 30}

# sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

new_order = [0, 1, 2, 3, 4, 5, 6, 7]


def get_image_files_from_folder(folder):
    # small helper function to get all the image files in a folder
    image_files = [f for f in os.listdir(folder) if
                   os.path.isfile(os.path.join(folder, f))]
    image_files.sort()
    image_files = [f for f in image_files if f.endswith(".tiff")]
    return image_files


def resort_image_wavelengths(collapsed_image):
    return collapsed_image[:, new_order]


def resort_wavelengths(msi):
    """Neil organized his _wavelengths differently.
    This function fixes this."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = resort_image_wavelengths(collapsed_image)
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the _wavelengths got messed up
sp.resort_wavelengths = resort_wavelengths

sp.bands_to_sortout = np.array([])


class ResultsFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER, "results.csv"))


class OxyAndVhbOverTimeTask(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "liver_oxy_over_time.pdf"))

    def requires(self):
        return ResultsFile()

    def run(self):
        df = pd.read_csv(self.input().path, index_col=0)
        # determine times from start:
        image_name_strings = df["image name"].values
        time_strings = map(lambda s: s[
            s.find("2014-08-03_")+11:s.find("2014-08-03_")+19],
                           image_name_strings)
        time_in_s = map(lambda s: int(s[0:2]) * 3600 +
                                  int(s[3:5]) * 60 +
                                  int(s[6:]), time_strings)
        df["time since drug delivery [s]"] = np.array(time_in_s) - time_in_s[0]

        # print oxy over time as scatterplot.
        ax = df.plot.scatter(x="time since drug delivery [s]",
                             y="oxygenation mean [%]",
                             s=100, alpha=0.5,
                             fontsize=30)
        ax.set_xlim((-1, 70))

        plt.axvline(x=0, ymin=0, ymax=1, linewidth=2)
        plt.axvline(x=56, ymin=0, ymax=1, linewidth=2)
        ax.annotate('drug delivery', xy=(0, ax.get_ylim()[1]),
                    xycoords='data', xytext=(0, 0),
                    fontsize=30,
                    textcoords='offset points')
        ax.annotate('porcine death', xy=(56, ax.get_ylim()[1]),
                    xycoords='data', xytext=(-100, 0),
                    fontsize=30,
                    textcoords='offset points')
        ax.yaxis.label.set_size(30)
        ax.xaxis.label.set_size(30)

        df.to_csv(self.input().path)

        # create and save vhb plot
        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight', mode="pdf")

        # print vhb over time as scatterplot.
        ax = df.plot.scatter(x="time since drug delivery [s]",
                             y="blood volume fraction mean [%]",
                             s=100, alpha=0.5,
                             fontsize=30)
        ax.set_xlim((-1, 70))

        plt.axvline(x=0, ymin=0, ymax=1, linewidth=2)
        plt.axvline(x=56, ymin=0, ymax=1, linewidth=2)
        ax.annotate('drug delivery', xy=(0, ax.get_ylim()[1]),
                    xycoords='data', xytext=(0, 0),
                    fontsize=30,
                    textcoords='offset points')
        ax.annotate('porcine death', xy=(56, ax.get_ylim()[1]),
                    xycoords='data', xytext=(-100, 0),
                    fontsize=30,
                    textcoords='offset points')
        ax.yaxis.label.set_size(30)
        ax.xaxis.label.set_size(30)

        plt.savefig(self.output().path + "_vhb_mean.pdf",
                    dpi=250, bbox_inches='tight', mode="pdf")


class IPCAICreateOxyImageTask(luigi.Task):
    image_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAITrainRegressor(df_prefix=self.df_prefix), \
               Flatfield(flatfield_folder=sp.FLAT_FOLDER), \
               SingleMultispectralImage(image=self.image_name), \
               Dark(dark_folder=sp.DARK_FOLDER)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.image_name + "_" +
                                              self.df_prefix +
                                              "_oxy_summary" + ".png"))

    def run(self):
        nrrd_reader = NrrdReader()
        tiff_ring_reader = TiffRingReader()
        # read the flatfield
        flat = nrrd_reader.read(self.input()[1].path)
        dark = nrrd_reader.read(self.input()[3].path)
        # read the msi
        nr_filters = len(sp.RECORDED_WAVELENGTHS)
        msi, segmentation = tiff_ring_reader.read(self.input()[2].path,
                                                  nr_filters)

        # only take into account not saturated pixels.
        segmentation = np.max(msi.get_image(), axis=-1) < 2000.

        # read the regressor
        e_file = open(self.input()[0].path, 'r')
        e = pickle.load(e_file)

        # correct image setup
        position_filter_nr_in_string = self.image_name.find(" 2014") - 1
        filter_nr = int(self.image_name[
                position_filter_nr_in_string:position_filter_nr_in_string+1])
        original_order = np.arange(nr_filters)
        new_image_order = np.concatenate((
                                original_order[nr_filters - filter_nr:],
                                original_order[:nr_filters - filter_nr]))
        # resort msi to restore original order
        msimani.get_bands(msi, new_image_order)
        # correct by flatfield
        msimani.image_correction(msi, flat, dark)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [2, 3, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.

        # preprocess the image
        # sortout unwanted bands
        print "1"
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
        print "2"
        # estimate
        sitk_image, time = estimate_image(msi, e)
        image = sitk.GetArrayFromImage(sitk_image)

        plt.figure()
        print "3"
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(5.)
        plot_image = np.array(im)
        top_left_axis = plt.gca()
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")
        print "4"
        # plot parametric maps
        # first oxygenation
        plt.figure()
        oxy_image = image[:, :, 0]
        segmentation[0, 0] = 1
        segmentation[0, 1] = 1
        oxy_image = np.ma.masked_array(oxy_image, ~segmentation)
        oxy_image[np.isnan(oxy_image)] = 0.
        oxy_image[np.isinf(oxy_image)] = 0.
        oxy_mean = np.mean(oxy_image)
        oxy_image[0, 0] = 0.0
        oxy_image[0, 1] = 1.
        at.plot_axis(plt.gca(), oxy_image,
                  "oxygenation [%]. Mean " +
                  "{0:.1f}".format((oxy_mean * 100.)) + "%")
        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight')
        # second blood volume fraction
        plt.figure()
        vhb_image = image[:, :, 1]
        vhb_image = np.ma.masked_array(vhb_image, ~segmentation)
        vhb_image[np.isnan(vhb_image)] = 0.
        vhb_image[np.isinf(vhb_image)] = 0.
        vhb_image[0, 0] = 0.0
        vhb_image[0, 1] = 0.1
        vhb_image = np.clip(vhb_image, 0.0, 0.1)
        vhb_mean = np.mean(vhb_image)
        at.plot_axis(plt.gca(), vhb_image,
                  "vhb [%]. Mean " +
                  "{0:.1f}".format((vhb_mean * 100.)) + "%")
        plt.savefig(self.output().path + "vhb.png",
                    dpi=250, bbox_inches='tight')

        # store results summary in dataframe
        df_image_results = pd.DataFrame(data=np.expand_dims([self.image_name,
                                                             oxy_mean * 100.,
                                                             vhb_mean * 100.,
                                                             time], 0),
                                        columns=["image name",
                                                 "oxygenation mean [%]",
                                                 "blood volume fraction mean [%]",
                                                 "time to estimate"])

        results_file = os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
                                    sp.FINALS_FOLDER, "results.csv")
        if os.path.isfile(results_file):
            df_results = pd.read_csv(results_file, index_col=0)
            df_results = pd.concat((df_results, df_image_results)).reset_index(
                drop=True
            )
        else:
            df_results = df_image_results

        df_results.to_csv(results_file)

#         plt.figure()
#         bvf_image = np.ma.masked_array(image[:, :, 0], (segmentation < 1))
#         # bvf_image = image[:, :, 0]
#         bvf_image[np.isnan(bvf_image)] = 0.
#         bvf_image[np.isinf(bvf_image)] = 0.
#         bvf_image[bvf_image > 1.] = 1.
#         bvf_image[bvf_image < 0.] = 0.
#         bvf_mean = np.mean(bvf_image)
#         bvf_image[0, 0] = 0.
#         bvf_image[1, 1] = 0.1
#         at.plot_axis(plt.gca(), bvf_image[:, :] * 100,
#                   "blood volume fraction [%]. Mean " + "{0:.1f}".format((bvf_mean * 100.)) + "%")
#
#         plt.savefig(self.output().path + "_bvf.png", dpi=500, bbox_inches='tight')
        print "4.5"

#         fd = open(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
#                                    sp.FINALS_FOLDER,
#                                    'results_summary.csv'), 'a')
#         fd.write(self.image_name + ", " + str(int(oxy_mean * 100)) +
#                  ", " + "{0:.1f}".format((bvf_mean * 100.)) + "\n")
#         fd.close()
        print "5"
        plt.close("all")


class IPCAITrainRegressor(luigi.Task):
    df_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "reg_" +
                                              self.df_prefix))

    def requires(self):
        return tasks_mc.CameraBatch(self.df_prefix)

    def run(self):
        # extract data from the batch
        df_train = pd.read_csv(self.input().path, header=[0, 1])
        df_snrs = pd.read_csv("/media/wirkert/data/Data/" +
                              "2016_01_08_Calibration_Measurements/processed/" +
                              "Carets_Laparoscope/finals/" +
                              "snr_for_bands.txt", index_col=0)

        X, y = preprocess2(df_train, snr=10.,
                           bands_to_sortout=sp.bands_to_sortout)
        # train regressor
        reg = RandomForestRegressor(10, min_samples_leaf=10, max_depth=9,
                                    n_jobs=-1)
        # reg = KNeighborsRegressor(algorithm="auto")
        # reg = LinearRegression()
        # reg = sklearn.svm.SVR(kernel="rbf", degree=3, C=100., gamma=10.)
        # reg = LinearSaO2Unmixing()
        reg.fit(X, y)
        # reg = LinearSaO2Unmixing()
        # save regressor
        regressor_file = self.output().open('w')
        pickle.dump(reg, regressor_file)
        regressor_file.close()


class SingleMultispectralImage(luigi.Task):

    image = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER, \
                                              self.image))


class Flatfield(luigi.Task):
    flatfield_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "flatfield" +
                                ".nrrd"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sp.RECORDED_WAVELENGTHS)

        # analyze all the first image files
        image_files = get_image_files_from_folder(self.flatfield_folder)
        image_files = filter(lambda image_name: "F0" in image_name, image_files)

        # helper function to take maximum of two images
        def maximum_of_two_images(image_1, image_name_2):
            image_2 = tiff_ring_reader.read(os.path.join(self.flatfield_folder,
                                                         image_name_2),
                                            nr_filters)[0].get_image()
            return np.maximum(image_1, image_2)

        # now reduce to maximum of all the single images
        flat_maximum = reduce(lambda x, y: maximum_of_two_images(x, y),
                              image_files, 0)
        msi = Msi(image=flat_maximum)
        msi.set_wavelengths(sp.RECORDED_WAVELENGTHS)

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


class Dark(luigi.Task):
    dark_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "dark" +
                                ".nrrd"))

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
                                tiff_ring_reader.read(os.path.join(self.dark_folder, image_name),
                                                      nr_filters)[0]),
                         image_files)
        dark_means_sum = reduce(lambda x, y: x+y.get_image(), dark_means, 0)
        final_dark_mean = dark_means_sum / len(dark_means)

        msi = Msi(image=final_dark_mean)
        msi.set_wavelengths(sp.RECORDED_WAVELENGTHS)

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


if __name__ == '__main__':

    # root folder there the data lies
    logging.basicConfig(filename='liver' + str(datetime.datetime.now()) +
                        '.log', level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    image_file_folder = os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER)
    onlyfiles = get_image_files_from_folder(image_file_folder)

    first_invivo_image_files = filter(lambda image_name: "0 2014" in image_name,
                                      onlyfiles)

    for f in first_invivo_image_files:
        main_task = IPCAICreateOxyImageTask(image_name=f,
                df_prefix="ipcai_revision_colon_mean_scattering_train")
        w.add(main_task)

    w.run()

    oxygenation_over_time_task = OxyAndVhbOverTimeTask()
    w.add(oxygenation_over_time_task)
    w.run()

