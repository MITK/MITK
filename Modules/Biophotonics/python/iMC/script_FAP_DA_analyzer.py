# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import luigi

import tasks_regression as rt
import tasks_preprocessing as ppt
import tasks_analyze as at
import scriptpaths as sp

sp.FINALS_FOLDER = "FAP_DA"


def resort_wavelengths(msi):
    """somehow the wavelength sorting got mixed up.
    This function fixes the mess."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = collapsed_image[:, [2, 1, 0, 3, 4, 5, 6, 7]]
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the wavelengths got messed up
ppt.resort_wavelengths = resort_wavelengths


class EstimateTissueParametersTaskDA(at.EstimateTissueParametersTask):

    def requires(self):
        return ppt.PreprocessMSI(imageName=self.imageName), \
            rt.TrainForestDA(imageName=self.imageName,
                        file_name_prefix_training=
                        self.file_name_prefix_training)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, "processed",
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.file_name_prefix_training +
                                              "estimate.nrrd"))



class CreateNiceParametricImagesTaskDA(at.CreateNiceParametricImagesTask):

    def requires(self):
        return EstimateTissueParametersTaskDA(imageName=self.imageName,
                                            file_name_prefix_training=
                                            self.file_name_prefix_training), \
            ppt.CorrectImagingSetupTask(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName), \
            ppt.PreprocessMSI(imageName=self.imageName), \
            at.ReprojectReflectancesTask(imageName=self.imageName,
                                      file_name_prefix_training=
                                      self.file_name_prefix_training)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.imageName + "_" +
                                              self.file_name_prefix_training +
                                              "_summary.png"))



if __name__ == '__main__':

    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    for root, dirs, files in os.walk(os.path.join(sp.ROOT_FOLDER,
                                                  sp.DATA_FOLDER)):
        for name in files:
            main_task = CreateNiceParametricImagesTaskDA(
                imageName=name,
                file_name_prefix_training=
                "2015June0809:51PMNoisyRandomTraining")
            w.add(main_task)
    w.run()


