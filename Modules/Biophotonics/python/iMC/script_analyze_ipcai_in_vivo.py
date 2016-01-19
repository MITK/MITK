# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import Image
import ImageEnhance
import pickle
import copy
import logging
import datetime

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
from msi.msi import Msi
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.normalize as norm
from regression.estimation import estimate_image
from regression.linear import LinearSaO2Unmixing
from regression.preprocessing import preprocess, normalize



sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_Neil_InVivo"
sp.DATA_FOLDER = "colon_images/not_registered"
sp.MC_DATA_FOLDER = "mc_data2"
sp.FINALS_FOLDER = "OxySecondCamera"

sp.RECORDED_WAVELENGTHS = np.arange(400, 730, 10) * 10 ** -9

def resort_image_wavelengths(collapsed_image):
    return collapsed_image

def resort_wavelengths(msi):
    """Neil organized his wavelengths differently.
    This function fixes this."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = resort_image_wavelengths(collapsed_image)
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
sp.resort_wavelengths = resort_wavelengths

sp.bands_to_sortout = np.array([0, 1, 2, 3, 4, 5, 6 , 7, 8, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32])



class IPCAICreateOxyImageTask(luigi.Task):
    folder_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAIEstimateTissueParametersTask(folder_name=self.folder_name,
                                            batch_prefix=
                                            self.df_prefix), \
            MultiSpectralImageFromPNGFiles(folder_name=self.folder_name)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.folder_name + "_" +
                                              self.df_prefix +
                                              "_summary.png"))

    def run(self):
        nrrdReader = NrrdReader()
        # read in the parametric image
        param_image = nrrdReader.read(self.input()[0].path)
        image = param_image.get_image()

        # read the multispectral image
        msi = nrrdReader.read(self.input()[1].path)

        # segmentation window:
        seg_windows = np.array([
            [135, 332, 227, 154 ],
            [434, 280, 161, 161 ],
            [428, 381, 99 , 99   ],
            [701, 368, 161, 148 ],
            [756, 363, 151, 139 ],
            [630, 340, 150, 129 ],
            [468, 505, 97 , 108  ],
            [679, 499, 114, 114 ],
            [493, 513, 89 , 89   ],
            [336, 72 , 177 , 152  ],
            [313, 122, 132, 139 ],
            [697, 311, 113, 103 ],
            [881, 400, 128, 112 ],
            [634, 452, 124, 103 ],
            [632, 452, 120, 108 ],
            [675, 478, 122, 126 ],
            [774, 577, 128, 126 ],
            [490, 501, 130, 99  ],
            [495, 597, 112, 91  ],
            [499, 482, 103, 95  ],
            [681, 408, 122, 85  ],
            [747, 439, 120, 99  ],
            [634, 509, 99 , 103  ],
            [571, 499, 108, 85  ],
            [760, 495, 97 , 85   ],
            [636, 415, 93 , 77   ],
            [872, 445, 112, 103 ],
            [659, 462, 118, 103 ],
            [610, 452, 89 , 73   ],
            [840, 616, 89 , 75   ],
            [620, 419, 97 , 81   ],
            [597, 404, 79 , 83   ],
            [657, 400, 97 , 81   ],
            [501, 341, 95 , 77   ],
            [612, 398, 89 , 83   ],
            [655, 437, 79 , 68   ],
            [776, 394, 89 , 83   ],
            [597, 462, 93 , 71   ],
            [681, 505, 81 , 81   ],
            [566, 427, 83 , 71   ],
            [727, 427, 81 , 69   ],
            [772, 486, 103, 89  ],
            [515, 511, 83 , 73   ],
            [673, 412, 108, 91  ],
            [895, 505, 101, 93  ],
            [474, 365, 120, 101 ]])


        # plot
        f, axarr = plt.subplots(1, 2)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [-1, 6, 0]]
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

        # postprocess parametric maps
        image[np.isnan(image)] = 0.
        image[np.isinf(image)] = 0.
        image[image > 1.] = 1.
        image[image < 0.] = 0.
        image[0, 0] = 0.
        image[0, 1] = 1.

        try:
            folder_nr = int(self.folder_name[-2:])
        except ValueError:
            folder_nr = int(self.folder_name[-1])
        seg_window = seg_windows[folder_nr - 1, :]
        evaluation_image = copy.deepcopy(image)
        # set out-segmented pixels to -1
        evaluation_image[(msi.get_image() >= np.max(msi.get_image())).any(axis=2)] = -1
        evaluation_image[(msi.get_image() <= 0).any(axis=2)] = -1
        # only take elements in window
        evaluation_image = evaluation_image[
                                    seg_window[1]:seg_window[1] + seg_window[3],
                                    seg_window[0]:seg_window[0] + seg_window[2]]
        # throw away elements not in window
        result = np.mean(evaluation_image[evaluation_image >= 0])
        logging.info("mean of image is: " +
                     str(result))
        fd = open(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
                                               sp.FINALS_FOLDER,
                                               'results_summary.csv'), 'a')
        fd.write(str(folder_nr) + ", " + str(result) + "\n")
        fd.close()

        # write the windowed evaluation image
        sitk_img = sitk.GetImageFromArray(evaluation_image,
                             isVector=True)
        sitk.WriteImage(sitk_img, self.output().path + "window.nrrd")

        # plot parametric maps
        at.plot_axis(axarr[1], image[:, :],
                  "oxygenation [%]")
        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')


class IPCAIEstimateTissueParametersTask(luigi.Task):
    folder_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAIPreprocessMSI(folder_name=self.folder_name), \
            IPCAITrainRegressor(batch_prefix=
                        self.df_prefix)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.folder_name + "_" +
                                              self.df_prefix +
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
        f = open(self.input().path, "r")
        batch_train = pickle.load(f)
        X, y = preprocess(batch_train,
                          w_percent=0.1, bands_to_sortout=sp.bands_to_sortout)

        # train regressor
        reg = RandomForestRegressor(max_depth=10, n_estimators=10,
                                    min_samples_leaf=1, n_jobs=-1)
        # reg = LinearSaO2Unmixing()
        reg.fit(X, y)
        # reg = LinearSaO2Unmixing()
        # save regressor
        f = self.output().open('w')
        pickle.dump(reg, f)
        f.close()


class IPCAIPreprocessMSI(luigi.Task):
    folder_name = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                  self.folder_name + "_preprocessed.nrrd"))

    def requires(self):
        return IPCAICorrectImagingSetupTask(folder_name=self.folder_name)

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
        pre.touch_and_save_msi(msi, self.output())


class IPCAICorrectImagingSetupTask(luigi.Task):
    """corrects for dark image
    and flatfield"""
    folder_name = luigi.Parameter()

    def requires(self):
        return MultiSpectralImageFromPNGFiles(folder_name=self.folder_name), \
            FlatfieldFromMeasurement()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                self.folder_name +
                                "_image_setup_corrected.nrrd"))

    def run(self):
        """sort wavelengths and normalize by respective integration times"""
        # read inputs
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        flatfield = reader.read(self.input()[1].path)
        # msi.set_image(gaussian_filter(msi.get_image().astype(np.float), (1, 1, 0)))
        # correct image by flatfield
        msimani.flatfield_correction(msi, flatfield)
        sp.resort_wavelengths(msi)
        pre.touch_and_save_msi(msi, self.output())


class FlatfieldFromMeasurement(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "flatfield" +
                                ".nrrd"))

    def run(self):
        msi = Msi()
        msi.set_image(np.array([
                                                   0.90733563 ,
                                                   3.6583764  ,
                                                   27.30637258,
                                                   52.35073931,
                                                   103.0408631,
                                                   176.812168 ,
                                                   392.361853 ,
                                                   620.0531964,
                                                   712.2926615,
                                                   1052.643461,
                                                   1202.266783,
                                                   1483.629833,
                                                   1730.168541,
                                                   2041.730527,
                                                   2296.256574,
                                                   2399.004343,
                                                   2536.580738,
                                                   2736.328832,
                                                   2760.046341,
                                                   2927.307208,
                                                   3539.747558,
                                                   3420.653065,
                                                   3229.74619 ,
                                                   3435.280705,
                                                   3108.530329,
                                                   2817.607701,
                                                   2612.795573,
                                                   2053.155475,
                                                   1343.305582,
                                                   721.058596 ,
                                                   340.6692005,
                                                   189.2180019,
                                                   77.61268264
]))
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


class MultiSpectralImageFromPNGFiles(luigi.Task):
    folder_name = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                self.folder_name +
                                ".nrrd"))

    def run(self):
        reader = PngReader()
        msi = reader.read(os.path.join(sp.ROOT_FOLDER,
                                       sp.DATA_FOLDER,
                                       self.folder_name,
                                       "image sample"))
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


if __name__ == '__main__':

    # configure logging
    logging.basicConfig(filename='invivo_neil_colon' +
                        str(datetime.datetime.now()) +
                         '.log', level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    # configure luigi
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    image_file_folder = os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER)
    all_folders = os.listdir(image_file_folder)
    # each complete stack has elements F0 - F7
    only_colon_folders = [ f for f in all_folders if
                 f.startswith("SaO2") ]

    for f in only_colon_folders:
        main_task = IPCAICreateOxyImageTask(
            folder_name=f,
            batch_prefix=
            "ipcai_colon_muscle_train")
        w.add(main_task)
    w.run()


