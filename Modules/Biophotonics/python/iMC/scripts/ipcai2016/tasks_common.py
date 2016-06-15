
import os
import pickle

import numpy as np
import pandas as pd
import luigi
from sklearn.ensemble.forest import RandomForestRegressor
import matplotlib.pylab as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable

import tasks_mc
import commons
from msi.msi import Msi
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
from regression.preprocessing import preprocess2
from msi.io.tiffringreader import TiffRingReader

sc = commons.ScriptCommons()

"""
Collection of functions and luigi.Task s which are used by more than one script
"""


def get_image_files_from_folder(folder,
                                prefix="", suffix=".tiff", fullpath=False):
    # small helper function to get all the image files in a folder
    # it will only return files which end with suffix.
    # if fullpath==True it will return the full path of the file, otherwise
    # only the filename
    # get all filenames
    image_files = [f for f in os.listdir(folder) if
                   os.path.isfile(os.path.join(folder, f))]
    image_files.sort()
    image_files = [f for f in image_files if f.endswith(suffix)]
    image_files = [f for f in image_files if f.startswith(prefix)]
    if fullpath:  # expand to full path if wished
        image_files = [os.path.join(folder, f) for f in image_files]
    return image_files


def plot_image(image, axis=None, title=None, cmap=None):
    if axis is None:
        axis = plt.gca()
    if cmap is None:
        im = axis.imshow(image, interpolation='nearest', alpha=1.0)
    else:
        im = axis.imshow(image, interpolation='nearest', alpha=1.0,
                            cmap=cmap)
        divider = make_axes_locatable(axis)
        cax = divider.append_axes("right", size="20%", pad=0.05)
        cbar = plt.colorbar(im, cax=cax)

    axis.xaxis.set_visible(False)
    axis.yaxis.set_visible(False)
    if title is not None:
        axis.set_title(title)


class IPCAITrainRegressor(luigi.Task):
    df_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("INTERMEDIATES_FOLDER"),
                                              "reg_small_bowel_" +
                                              self.df_prefix))

    def requires(self):
        return tasks_mc.SpectroCamBatch(self.df_prefix)

    def run(self):
        # extract data from the batch
        df_train = pd.read_csv(self.input().path, header=[0, 1])

        X, y = preprocess2(df_train, snr=10.)
        # train regressor
        reg = RandomForestRegressor(10, min_samples_leaf=10, max_depth=9,
                                    n_jobs=-1)
        # reg = KNeighborsRegressor(algorithm="auto")
        # reg = LinearRegression()
        # reg = sklearn.svm.SVR(kernel="rbf", degree=3, C=100., gamma=10.)
        # reg = LinearSaO2Unmixing()
        reg.fit(X, y.values)
        # reg = LinearSaO2Unmixing()
        # save regressor
        regressor_file = self.output().open('w')
        pickle.dump(reg, regressor_file)
        regressor_file.close()


class SingleMultispectralImage(luigi.Task):

    image = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(self.image)


class Flatfield(luigi.Task):

    flatfield_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("INTERMEDIATES_FOLDER"),
                                "flatfield.nrrd"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])

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
        msi.set_wavelengths(sc.other["RECORDED_WAVELENGTHS"])

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


class Dark(luigi.Task):
    dark_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("INTERMEDIATES_FOLDER"),
                                "dark" +
                                ".nrrd"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])

        # analyze all the first image files
        image_files = get_image_files_from_folder(self.dark_folder,
                                                  suffix="F0.tiff")

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
        msi.set_wavelengths(sc.other["RECORDED_WAVELENGTHS"])

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)
