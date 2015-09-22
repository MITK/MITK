'''
Created on Sep 21, 2015

@author: wirkert
'''

import logging
import os

import matplotlib.pyplot as plt
import numpy as np
import luigi

import scriptpaths as sp
import tasks_preprocessing as pre
import msi.imgmani as imgmani
from msi.io.nrrdreader import NrrdReader
import msi.plot as msiplt
from msi.msi import Msi

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_xxx_ambient_light"


def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
pre.resort_wavelengths = resort_wavelengths


class WhiteSegmentationFile(luigi.Task):
    imageName = luigi.Parameter()

    def output(self):
        white_seg_imagename = \
            self.imageName.replace(".nrrd",
                                   "_white_segmentation.nrrd")
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                      sp.DATA_FOLDER,
                        white_seg_imagename))



class PlotAmbientLight(luigi.Task):
    imageName = luigi.Parameter()

    def requires(self):
        return pre.MultiSpectralImageFile(self.imageName), \
            pre.PreprocessFlatfields(), \
            WhiteSegmentationFile(imageName=self.imageName),

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                self.imageName +
                                "_ambient_light_plot.png"))

    def run(self):
        reader = NrrdReader()
        msi = reader.read(self.input()[0].path)
        flatfield = reader.read(self.input()[1].path)
        segmentation = reader.read(self.input()[-1].path)
        af = pre.get_ambient_factor(flatfield, msi, segmentation)
        af_msi = Msi()
        af_msi.set_image(af)
        af_msi.set_wavelengths(sp.RECORDED_WAVELENGTHS)
        pre.resort_wavelengths(af_msi)
        plt.figure()
        msiplt.plot(af_msi)
        plt.grid()
        plt.gca().set_ylim([0.6, 1.2])
        plt.savefig(self.output().path, dpi=250)




if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    image_nrs = np.arange(1, 25, 1)
    for i in image_nrs:
        main_task = PlotAmbientLight(imageName="FAP" + str(i) + ".nrrd")
        w.add(main_task)
    w.run()
