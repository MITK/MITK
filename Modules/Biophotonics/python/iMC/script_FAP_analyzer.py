# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import numpy as np

import luigi

import msi.imgmani as imgmani
import analyzetasks as at
import preproctasks as ppt
import scriptpaths as sp

sp.FINALS_FOLDER = "FAP"

def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
ppt.resort_wavelengths = resort_wavelengths


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
            main_task = at.CreateNiceParametricImagesTask(
                imageName=name,
                file_name_prefix_training=
                "2015August2811:59PMNoisyRandomForInvertabilityTraining")
            w.add(main_task)
    w.run()


