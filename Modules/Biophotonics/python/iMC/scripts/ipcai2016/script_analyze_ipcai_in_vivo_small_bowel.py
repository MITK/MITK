# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import Image
import ImageEnhance
import logging
import datetime

import SimpleITK as sitk
import matplotlib

from msi.io.nrrdreader import NrrdReader
import msi.normalize as norm
from regression.estimation import estimate_image
from tasks_common import *
import commons
from msi.io.tiffringreader import TiffRingReader

TiffRingReader.RESIZE_FACTOR = 0.5

sc = commons.ScriptCommons()


sc.add_dir("SMALL_BOWEL_DATA",
           os.path.join(sc.get_dir("DATA_FOLDER"), "small_bowel_images"))

sc.add_dir("SMALL_BOWEL_RESULT", os.path.join(sc.get_dir("RESULTS_FOLDER"),
                                              "small_bowel"))

sc.add_dir("FILTER_TRANSMISSIONS",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "filter_transmissions"))

font = {'family' : 'normal',
        'size'   : 25}
matplotlib.rc('font', **font)


class ResultsFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(
                sc.get_full_dir("SMALL_BOWEL_RESULT"), "results.csv"))


class OxyOverTimeTask(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(
                sc.get_full_dir("SMALL_BOWEL_RESULT"),
                "colon_oxy_over_time.pdf"))

    def requires(self):
        return ResultsFile()

    def run(self):
        df = pd.read_csv(self.input().path, index_col=0)

        # determine times from start:
        image_name_strings = df["image name"].values
        time_strings = map(lambda s: s[
            s.find("2015-10-08_")+11:s.find("2015-10-08_")+19],
                           image_name_strings)
        time_in_s = map(lambda s: int(s[0:2]) * 3600 +
                                  int(s[3:5]) * 60 +
                                  int(s[6:]), time_strings)
        df["time since first frame [s]"] = np.array(time_in_s) - time_in_s[0]
        df["time since applying first clip [s]"] = df["time since first frame [s]"] - 4
        # print oxy over time as scatterplot.
        plt.figure()
        ax = df.plot.scatter(x="time since applying first clip [s]",
                             y="oxygenation mean [%]", fontsize=30,
                             s=50, alpha=0.5)
        ax.set_xlim((-5, 600))

        plt.axvline(x=0, ymin=0, ymax=1, linewidth=2)
        plt.axvline(x=39, ymin=0, ymax=1, linewidth=2)
        plt.axvline(x=100, ymin=0, ymax=1, linewidth=2)
        plt.axvline(x=124, ymin=0, ymax=1, linewidth=2)
        ax.annotate('1', xy=(0, ax.get_ylim()[1]),
                    fontsize=18,
                    color="blue",
                    xycoords='data', xytext=(-5, 0),
                    textcoords='offset points')
        ax.annotate('2', xy=(39, ax.get_ylim()[1]),
                    fontsize=18,
                    color="blue",
                    xycoords='data', xytext=(-5, 0),
                    textcoords='offset points')
        ax.annotate('3', xy=(100, ax.get_ylim()[1]),
                    fontsize=18,
                    color="blue",
                    xycoords='data', xytext=(-5, 0),
                    textcoords='offset points')
        ax.annotate('4', xy=(124, ax.get_ylim()[1]),
                    fontsize=18,
                    color="blue",
                    xycoords='data', xytext=(-5, 0),
                    textcoords='offset points')

        plt.grid()

        df.to_csv(self.input().path)

        # save
        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight', mode="pdf")


def plot_image(image, axis):
    im2 = axis.imshow(image, interpolation='nearest', alpha=1.0)
    # axis.set_title(title, fontsize=5)
    # divider = make_axes_locatable(axis)
    # cax = divider.append_axes("right", size="10%", pad=0.05)
    # cbar = plt.colorbar(im2, cax=cax)
    # cbar.ax.tick_params(labelsize=5)
    axis.xaxis.set_visible(False)


class IPCAICreateOxyImageTask(luigi.Task):
    image_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAITrainRegressor(df_prefix=self.df_prefix), \
               Flatfield(flatfield_folder=sc.get_full_dir("FLAT_FOLDER")), \
               SingleMultispectralImage(image=self.image_name), \
               Dark(dark_folder=sc.get_full_dir("DARK_FOLDER"))

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("SMALL_BOWEL_RESULT"),
                                              os.path.split(self.image_name)[1] +
                                              "_" + self.df_prefix +
                                              "_summary" + ".png"))

    def run(self):
        nrrd_reader = NrrdReader()
        tiff_ring_reader = TiffRingReader()
        # read the flatfield
        flat = nrrd_reader.read(self.input()[1].path)
        dark = nrrd_reader.read(self.input()[3].path)
        # read the msi
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])
        msi, segmentation = tiff_ring_reader.read(self.input()[2].path,
                                                  nr_filters)
        # only take into account not saturated pixels.
        segmentation = np.logical_and(segmentation,
                                      (np.max(msi.get_image(),
                                              axis=-1) < 1000.))

        # read the regressor
        e_file = open(self.input()[0].path, 'r')
        e = pickle.load(e_file)

        # correct image setup
        filter_nr = int(self.image_name[-6:-5])
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
        im = enh_brightness.enhance(10.)
        plotted_image = np.array(im)
        top_left_axis = plt.gca()
        top_left_axis.imshow(plotted_image, interpolation='nearest')
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")
        print "4"
        # plot parametric maps
        segmentation[0, 0] = 1
        segmentation[0, 1] = 1
        oxy_image = np.ma.masked_array(image[:, :, 0], ~segmentation)
        oxy_image[np.isnan(oxy_image)] = 0.
        oxy_image[np.isinf(oxy_image)] = 0.
        oxy_mean = np.mean(oxy_image)
        oxy_image[0, 0] = 0.0
        oxy_image[0, 1] = 1.

        plot_image(oxy_image[:, :], plt.gca())

        df_image_results = pd.DataFrame(data=np.expand_dims([self.image_name,
                                                             oxy_mean * 100.,
                                                             time], 0),
                                        columns=["image name",
                                                 "oxygenation mean [%]",
                                                 "time to estimate"])

        results_file = os.path.join(sc.get_full_dir("SMALL_BOWEL_RESULT"),
                                    "results.csv")
        if os.path.isfile(results_file):
            df_results = pd.read_csv(results_file, index_col=0)
            df_results = pd.concat((df_results, df_image_results)).reset_index(
                drop=True
            )
        else:
            df_results = df_image_results

        df_results.to_csv(results_file)

        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight')
        plt.close("all")


if __name__ == '__main__':

    # create a folder for the results if necessary
    sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
    sc.create_folders()

    # root folder there the data lies
    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                 "small_bowel_images" +
                                 str(datetime.datetime.now()) +
                                 '.log'),
                        level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)


    # determine files to process
    files = get_image_files_from_folder(sc.get_full_dir("SMALL_BOWEL_DATA"),
                                        suffix="F0.tiff", fullpath=True)

    for f in files:
        main_task = IPCAICreateOxyImageTask(image_name=f,
                df_prefix="ipcai_revision_colon_mean_scattering_te")
        w.add(main_task)
    w.run()

    oxygenation_over_time_task = OxyOverTimeTask()
    w.add(oxygenation_over_time_task)
    w.run()

