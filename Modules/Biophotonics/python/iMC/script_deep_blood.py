'''
Created on Sep 21, 2015

@author: wirkert
'''

import os
import numpy as np
import Image
import ImageEnhance

import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
import luigi

import msi.imgmani as imgmani
import msi.msimanipulations as msimani
import tasks_preprocessing as pre
import msi.normalize as norm
import scriptpaths as sp
from msi.io.nrrdreader import NrrdReader

sp.FINALS_FOLDER = "DeepBlood"

def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
pre.resort_wavelengths = resort_wavelengths


class DeepBloodTask(luigi.Task):
    imageName = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                self.imageName +
                                "_deep_blood.nrrd"))

    def requires(self):
        return pre.CorrectImagingSetupTask(imageName=self.imageName)

    def run(self):
        # read inputs
        reader = NrrdReader()
        msi = reader.read(self.input().path)
        dp_normalizer = norm.NormalizeIQ()
        dp_normalizer.iqBand = 0
        dp_normalizer.normalize(msi)
        msi.set_image(imgmani.get_bands(msi.get_image(), -1))
        pre.touch_and_save_msi(msi, self.output())


class PlotDeepBlood(luigi.Task):
    imageName = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                self.imageName +
                                "_deep_blood.png"))

    def requires(self):
        return DeepBloodTask(imageName=self.imageName), \
            pre.CorrectImagingSetupTask(imageName=self.imageName)

    def run(self):
        # read inputs
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        rgb = reader.read(self.input()[1].path)
        # set extrem values
        image = msi.get_image()
        image[image < 0] = 0
        image[image > 25] = 25
        # plot dat
        f, axarr = plt.subplots(1, 2)
            # create artificial rgb
        rgb_image = rgb.get_image()[:, :, [7, 5, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(5.)
        plot_image = np.array(im)
        top_left_axis = axarr[0]
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.set_title("synthetic rgb",
                                fontsize=5)
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)
        # plot deep blood map
        axis = axarr[1]
        plt.set_cmap("jet")
        im2 = axis.imshow(image, interpolation='nearest', alpha=1.0)
        axis.set_title("deep blood map", fontsize=5)
        divider = make_axes_locatable(axis)
        cax = divider.append_axes("right", size="10%", pad=0.05)
        cbar = plt.colorbar(im2, cax=cax)
        cbar.ax.tick_params(labelsize=5)
        axis.xaxis.set_visible(False)
        axis.yaxis.set_visible(False)
        plt.savefig(self.output().path, dpi=1000, bbox_inches='tight')




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
    only_long_exposed_files = [ f for f in onlyfiles if
                 f.endswith("_long.nrrd") ]
#     main_task = ppt.PreprocessMSI(imageName="FAP7_long.nrrd")
    for long_exposed_file in only_long_exposed_files:
        main_task = PlotDeepBlood(
            imageName=long_exposed_file)
        w.add(main_task)
    w.run()
