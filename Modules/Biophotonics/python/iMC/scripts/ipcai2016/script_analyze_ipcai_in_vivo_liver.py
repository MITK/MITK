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

sc.add_dir("LIVER_DATA",
           os.path.join(sc.get_dir("DATA_FOLDER"), "liver_images"))
sc.add_dir("LIVER_RESULTS", os.path.join(sc.get_dir("RESULTS_FOLDER"), "liver"))

sc.add_dir("FILTER_TRANSMISSIONS",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "filter_transmissions"))

font = {'family' : 'normal',
        'size'   : 30}
matplotlib.rc('font', **font)


class ResultsFile(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("LIVER_RESULTS"),
                                              "results.csv"))


class OxyAndVhbOverTimeTask(luigi.Task):

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("LIVER_RESULTS"),
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
        plt.grid()

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
        plt.grid()

        plt.savefig(self.output().path + "_vhb_mean.pdf",
                    dpi=250, bbox_inches='tight', mode="pdf")


class IPCAICreateOxyImageTask(luigi.Task):
    image_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAITrainRegressor(df_prefix=self.df_prefix), \
               Flatfield(flatfield_folder=sc.get_full_dir("FLAT_FOLDER")), \
               SingleMultispectralImage(image=os.path.join(
                       sc.get_full_dir("LIVER_DATA"), self.image_name)), \
               Dark(dark_folder=sc.get_full_dir("DARK_FOLDER"))

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("LIVER_RESULTS"),
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
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])
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
        plotted_image = np.array(im)
        top_left_axis = plt.gca()
        top_left_axis.imshow(plotted_image, interpolation='nearest')
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
        plot_image(oxy_image[:, :], plt.gca())
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
        plot_image(vhb_image, plt.gca())
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

        results_file = os.path.join(sc.get_full_dir("LIVER_RESULTS"),
                                    "results.csv")
        if os.path.isfile(results_file):
            df_results = pd.read_csv(results_file, index_col=0)
            df_results = pd.concat((df_results, df_image_results)).reset_index(
                drop=True
            )
        else:
            df_results = df_image_results

        df_results.to_csv(results_file)

        print "5"
        plt.close("all")


if __name__ == '__main__':

    # create a folder for the results if necessary
    sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
    sc.create_folders()

    # root folder there the data lies
    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                 "liver" +
                                 str(datetime.datetime.now()) +
                                 '.log'), level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    onlyfiles = get_image_files_from_folder(sc.get_full_dir("LIVER_DATA"))

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

