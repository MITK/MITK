# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import pickle
import copy
from collections import namedtuple
import itertools

import numpy as np
import luigi
import matplotlib.pyplot as plt
from sklearn.preprocessing import Normalizer
from sklearn.linear_model import LinearRegression, LassoCV, LarsCV, ElasticNetCV, BayesianRidge, ARDRegression


import tasks_mc
import scriptpaths as sp
import mc.mcmanipulations as mcmani
from regression.linear import LinearSaO2Unmixing
from regression.domain_adaptation import estimate_logistic_regressor, resample
from regression.estimation import standard_score

sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_in_silico"
sp.FINALS_FOLDER = "Images"
sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

w_standard = 0.15  # for this evaluation we add 15% noise

def extract_batch_data(batch, nr_samples=None, w_percent=None):
    working_batch = copy.deepcopy(batch)
    if w_percent is None:
        w_percent = 0.
    # extract nr_samples samples from data
    if nr_samples is None:
        nr_samples = working_batch.nr_elements()
    mcmani.select_n(working_batch, nr_samples)
    # get reflectance and oxygenation
    reflectances = working_batch.reflectances
    y = working_batch.layers[0][:, 1]
    # add noise to reflectances
    if not np.isclose(w_percent, 0.):
        noises = np.random.normal(loc=0., scale=w_percent, size=reflectances.shape)
        reflectances += noises * reflectances
    reflectances = np.clip(reflectances, 0.00001, 1.)
    # normalize reflectances
    normalizer = Normalizer(norm='l1')
    reflectances = normalizer.transform(reflectances)
    # reflectances to absorption
    absorptions = -np.log(reflectances)
    X = absorptions
    # get rid of sorted out bands
    X = np.delete(X, sp.bands_to_sortout, axis=1)
    return X, y


class IPCAICreateInSilicoPlots(luigi.Task):

#     def requires(self):
#         return tasks_mc.CameraBatch(self.batch_prefix)
    def requires(self):
        return TrainingSamplePlots(), NoisePlots()


    def run(self):
        pass


EvaluationStruct = namedtuple("EvaluationStruct",
                              "regressor data median prctile25 prctile75 color alpha")


class TrainingSamplePlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_train"), \
                tasks_mc.CameraBatch("generic_tissue_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "samples_plot.png"))
    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)
        # setup testing function and data
        X_test, y_test = extract_batch_data(batch_test, w_percent=w_standard)
        X_train, y_train = extract_batch_data(batch_train, w_percent=w_standard)
        # setup structures to save test data
        evaluation_setups = [EvaluationStruct(LinearSaO2Unmixing(), [],
                                              [], [], [],
                                              "red", 0.25),
                     EvaluationStruct(LassoCV(), [], [], [], [],
                                      "green", 0.5)]
        # do  validation
        nr_training_samples = np.arange(10, 1005, 10).astype(int)
        # not very pythonic, don't care
        for n in nr_training_samples:
            print "nr samples ", str(n)
            nr_sets = X_train.shape[0] / n
            for e in evaluation_setups:
                median_sets = []
                for j in range(nr_sets):
                    X_j = X_train[j * n:(j * n) + n, :]
                    y_j = y_train[j * n:(j * n) + n]
                    lr = e.regressor
                    lr.fit(X_j, y_j)
                    # save results
                    median_sets.append(standard_score(lr, X_test, y_test))
                median_sets = np.array(median_sets)
                e.median.append(np.median(median_sets))
                e.prctile25.append(np.percentile(median_sets, 25))
                e.prctile75.append(np.percentile(median_sets, 75))
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            plt.fill_between(nr_training_samples,
                             np.array(e.prctile25) * 100.,
                             np.array(e.prctile75) * 100.,
                             edgecolor=e.color, facecolor=e.color,
                             alpha=e.alpha)
            h, = plt.plot(nr_training_samples,
                          np.array(e.median) * 100., color=e.color,
                          label=e.regressor.__class__.__name__ +
                            " final error: " + "{0:.2f}".format(e.median[-1] *
                                                                100.))
            legend_handles.append(h)
        minor_ticks_x = np.arange(0, 1000, 50)
        minor_ticks_y = np.arange(5, 16, 0.5)
        plt.gca().set_xticks(minor_ticks_x, minor=True)
        plt.gca().set_yticks(minor_ticks_y, minor=True)
        plt.grid()
        plt.legend(handles=legend_handles)
        plt.title("dependency on training samples")
        plt.xlabel("nr training samples")
        plt.ylabel("absolute error [%]")
        plt.savefig(self.output().path + "_temp.png",
                    dpi=500, bbox_inches='tight')


class NoisePlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_train"), \
                tasks_mc.CameraBatch("generic_tissue_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_plot.png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)
        # setup structures to save test data
        evaluation_setups = [EvaluationStruct(LinearSaO2Unmixing(), [],
                                              [], [], [],
                                              "red", 0.25),
                     EvaluationStruct(LassoCV(), [], [], [], [],
                                      "green", 0.5)]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 1.01, 0.01)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = extract_batch_data(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = extract_batch_data(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
                y_pred = lr.predict(X_test)
                # save results
                abs_errors = np.abs(y_pred - y_test)
                # save results
                e.median.append(np.median(abs_errors))
                e.prctile25.append(np.percentile(abs_errors, 25))
                e.prctile75.append(np.percentile(abs_errors, 75))
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            plt.fill_between(np.array(noise_levels) * 100.,
                             np.array(e.prctile25) * 100.,
                             np.array(e.prctile75) * 100.,
                             edgecolor=e.color, facecolor=e.color,
                             alpha=e.alpha)
            h, = plt.plot(np.array(noise_levels) * 100.,
                          np.array(e.median) * 100., color=e.color,
                          label=e.regressor.__class__.__name__)
            legend_handles.append(h)
        minor_ticks = np.arange(0, 100, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)
        plt.grid()
        plt.legend(handles=legend_handles)
        plt.title("dependency on noise")
        plt.xlabel("approach")
        plt.ylabel("absolute error [%]")
        plt.savefig(self.output().path + "_temp.png", dpi=500,
                    bbox_inches='tight')


class DAPlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_train"), \
                tasks_mc.CameraBatch("generic_tissue_test"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_train"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "covariance_shift_plot.png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)
        f = open(self.input()[2].path, "r")
        batch_da_train = pickle.load(f)
        f = open(self.input()[3].path, "r")
        batch_da_test = pickle.load(f)
        X_test, y_test = extract_batch_data(batch_test, w_percent=w_standard)
        X_train, y_train = extract_batch_data(batch_train, w_percent=w_standard)
        X_da_train, y_da_train = extract_batch_data(batch_da_train,
                                                    w_percent=w_standard)
        X_da_test, y_da_test = extract_batch_data(batch_da_test,
                                                  w_percent=w_standard)
        # setup structures to save test data
        evaluation_setups = [EvaluationStruct(LinearSaO2Unmixing(), [],
                                              [], [], [],
                                              "red", 0.25, "classic"),
                     EvaluationStruct(LassoCV(), [],
                                      [], [], [],
                                      "green", 0.5, "lasso"),
                     EvaluationStruct(LassoCV(), [],
                                      [], [], [],
                                      "green", 0.5, "lasso+DA")]
        # estimate weights for domain adaptation
        lr = estimate_logistic_regressor(X_train, X_da_train)
        weights = lr.predict_proba(X_train)[:, 1] / lr.predict_proba(X_train)[:, 0]
        # resample from X according da weighting result
        X_resampled, y_resampled, weights = resample(X_train, y_train, weights)
        for e in evaluation_setups:
            lr = e.regressor
            lr.fit(X_train, y_train)
            y_pred = lr.predict(X_da_test)
            # save results
            e.data.append(y_pred - y_da_test)


        # collect data
        boxplot_data = []
        for e in evaluation_setups:
            boxplot_data.append(100. * np.abs(np.array(e.data)))
        # make a nice plot for results
        fig = plt.figure()
        ax = fig.add_subplot(111)
        bp = ax.boxplot(boxplot_data, patch_artist=True)
        # nicen it up
        for e, box in zip(evaluation_setups, bp['boxes']):
            # change outline color
            box.set(color="black", linewidth=2)
            # change fill color
            box.set(facecolor=e.color, alpha=0.5)

        # change color and linewidth of the whiskers
        x_tick_labels = []
        for whisker, cap, median, flier in itertools.izip(bp['whiskers'], bp['caps'],
                              bp['medians'], evaluation_setups, bp['fliers']):
            whisker.set(color="black", linewidth=2)
            cap.set(color="black", linewidth=2)
            median.set(color="black", linewidth=2)
            flier.set(marker='o', color=e.color, alpha=0.5)
            x_tick_labels.append(e.regressor.__class__.__name__)
        ax.set_xticklabels(x_tick_labels)
        ax.get_xaxis().tick_bottom()
        ax.get_yaxis().tick_left()
        plt.grid()
        plt.title("performance under covariance shift")
        plt.xlabel("method")
        plt.ylabel("absolute error [%]")
        plt.savefig(self.output().path + "_temp.png", dpi=500,
                    bbox_inches='tight')


if __name__ == '__main__':
    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    main_task = DAPlots()
    w.add(main_task)
    w.run()

