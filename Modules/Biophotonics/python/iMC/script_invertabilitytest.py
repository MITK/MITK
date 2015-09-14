'''
Created on Aug 24, 2015

@author: wirkert
'''


import os
import pickle
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

import luigi
import tasks_regression as rt
import scriptpaths as sp

sp.FINALS_FOLDER = "invertabilitytest"





class TestInvertability(luigi.Task):
    batch_prefix = luigi.Parameter()
    file_name_prefix_testing = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "invertability_" +
                                              self.file_name_prefix_testing +
                                              ".png"))

    def requires(self):
        return rt.ParameterFile(self.file_name_prefix_testing), \
            rt.ReflectanceFile(self.file_name_prefix_testing), \
            rt.TrainForest(file_name_prefix_training=
                           self.batch_prefix), \
            rt.TrainForestAugmentedData(file_name_prefix_training=
                           self.batch_prefix)


    def run(self):
        # get the parameters to test
        test_params, test_refl = rt.extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        test_params = test_params[:, 0:4]
        # use regressors to calculate the errors
        # first random forest
        rf_regressor_file = self.input()[2].open('r')
        rf_regressor = pickle.load(rf_regressor_file)
        rf_absErrors = np.abs(rf_regressor.predict(test_refl) - test_params)
        rf_r2Score = rf_regressor.score(test_refl, test_params)
        # now augmented rf
        rf_aug_regressor_file = self.input()[3].open('r')
        rf_aug_regressor = pickle.load(rf_aug_regressor_file)
        rf_aug_absErrors = np.abs(rf_aug_regressor.predict(test_refl) - test_params)
        rf_aug_r2Score = rf_aug_regressor.score(test_refl, test_params)

        rf_r2Scores = [rf_r2Score, rf_aug_r2Score]

        # now for the plotting part
        colors = ['cyan', 'magenta']
        matplotlib.rcParams.update({'font.size': 5})
        f, axarr = plt.subplots(1, 4)
        bvf_axis = axarr[0]
        dsp_axis = axarr[1]
        r_axis = axarr[2]
        sao2_axis = axarr[3]

        boxplots = []

        data = [rf_absErrors[:, 0] , rf_aug_absErrors[:, 0]]
        boxplots.append(bvf_axis.boxplot(data, 0, "", patch_artist=True))
        bvf_axis.set_title("blood volume fraction")
        bvf_axis.set_ylabel('absolute error')
        bvf_axis.get_xaxis().set_ticks([])

        data = [rf_absErrors[:, 1], rf_aug_absErrors[:, 1]]
        boxplots.append(dsp_axis.boxplot(data, 0, "", patch_artist=True))
        dsp_axis.set_title("density scattering particles")
        dsp_axis.get_xaxis().set_ticks([])

        data = [rf_absErrors[:, 2], rf_aug_absErrors[:, 2]]
        boxplots.append(r_axis.boxplot(data, 0, "",
                                       patch_artist=True))
        r_axis.set_title("mucosa thickness")
        r_axis.get_xaxis().set_ticks([])

        data = [rf_absErrors[:, 3], rf_aug_absErrors[:, 3]]
        boxplots.append(sao2_axis.boxplot(data, 0, "", patch_artist=True))
        sao2_axis.set_title("oxygenation")
        sao2_axis.get_xaxis().set_ticks([])

        for boxplot in boxplots:
            for i, (patch, color) in enumerate(zip(boxplot['boxes'], colors)):
                patch.set_facecolor(color)
                head, filename = os.path.split(self.input()[2 + i].path)
                plt.figtext(0.30, 0.08 - 0.02 * i,
                    filename + ". rf_r2Score=" + str(rf_r2Scores[i]),
                    backgroundcolor=colors[i], color='black', weight='roman',
                    size='x-small')

        print("absolute error distribution BVF, Volume fraction")
        print("median: " + str(np.median(rf_absErrors, axis=0)))
        print("lower quartile: " + str(np.percentile(rf_absErrors,
                                                     25, axis=0)))
        print("higher quartile: " + str(np.percentile(rf_absErrors,
                                                      75, axis=0)))
        print("rf_r2Score", str(rf_r2Score))

        plt.savefig(self.output().path, dpi=500)


if __name__ == '__main__':

    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    main_task = TestInvertability(
                    file_name_prefix_training=
                    "2015June0809:51PMNoisyRandomTraining",
                    file_name_prefix_testing=
                    "2015June0902:43AMNoisyRandomTesting")

    w.add(main_task)
    w.run()

