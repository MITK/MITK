# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import numpy as np

import luigi

import msi.imgmani as imgmani
import tasks_analyze as at
import tasks_preprocessing as ppt
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
    image_file_folder = os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER)
    onlyfiles = [ f for f in os.listdir(image_file_folder) if
                 os.path.isfile(os.path.join(image_file_folder, f)) ]
    only_long_exposed_files = [ f for f in onlyfiles if
                 f.endswith("_long.nrrd") ]
#     main_task = ppt.PreprocessMSI(imageName="FAP7_long.nrrd")
    for long_exposed_file in only_long_exposed_files:
        main_task = at.CreateNiceParametricImagesTask(
            imageName=long_exposed_file,
            batch_prefix=
            "batch")
        w.add(main_task)
    w.run()


