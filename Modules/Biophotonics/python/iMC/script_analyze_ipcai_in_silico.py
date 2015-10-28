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
from sklearn.linear_model import LassoCV


import tasks_mc
import scriptpaths as sp
from regression.preprocessing import preprocess
import mc.mcmanipulations as mcmani
from regression.linear import LinearSaO2Unmixing
from regression.domain_adaptation import estimate_logistic_regressor, resample
from regression.estimation import standard_score
from sklearn.ensemble.forest import RandomForestRegressor
import sklearn.svm
import sklearn.grid_search
import sklearn.cross_validation
import pyRULSIF
from sklearn.decomposition import PCA
import sklearn.preprocessing
from sklearn.linear_model.base import LinearRegression

sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_in_silico"
sp.FINALS_FOLDER = "Images"
sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

w_standard = 0.05  # for this evaluation we add 5% noise


class IPCAICreateInSilicoPlots(luigi.Task):

    def requires(self):
        return TrainingSamplePlots(), NoisePlots()

    def run(self):
        pass

EvaluationStruct = namedtuple("EvaluationStruct",
                              "name regressor data color alpha")


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
        X_test, y_test = preprocess(batch_test, w_percent=w_standard)
        X_train, y_train = preprocess(batch_train, w_percent=w_standard)
        # setup structures to save test data
        evaluation_setups = [
                    EvaluationStruct("classic", LinearSaO2Unmixing(),
                                     [], "red", 0.25),
                    EvaluationStruct("lasso", LassoCV(), [], "yellow", 0.5)]
        # do  validation
        nr_training_samples = np.arange(10, 1010, 10).astype(int)
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
                e.data.append(np.array(median_sets))
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            plt.fill_between(nr_training_samples,
                             np.percentile(e.data[0], 25) * 100.,
                             np.percentile(e.data[0], 75) * 100.,
                             edgecolor=e.color, facecolor=e.color,
                             alpha=e.alpha)
            h, = plt.plot(nr_training_samples,
                          np.median(e.data[0]) * 100., color=e.color,
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


class WrongNoisePlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_no_aray_train"), \
                tasks_mc.CameraBatch("generic_tissue_no_aray_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_wrong_plot.png"))


    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)

        # setup structures to save test data
        kf = sklearn.cross_validation.KFold(batch_train.reflectances.shape[0],
                                            10, shuffle=True)
        param_grid_rf = [
          {"n_estimators": np.array([50]),
           "max_depth": np.array([30]),  # np.arange(30, 35, 2),
           "min_samples_leaf": np.array([5])}]
        rf = sklearn.grid_search.GridSearchCV(RandomForestRegressor(50,
                                                    max_depth=10, n_jobs=-1),
                  param_grid_rf, cv=kf, n_jobs=-1)

        evaluation_setups = [
                        EvaluationStruct("classic", LinearSaO2Unmixing(), [],
                                         "red", 0.25)
                        , EvaluationStruct("lasso", LassoCV(), [],
                                           "yellow", 0.5)

                        , EvaluationStruct("rf", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 1.00, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=0.1)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
                if e.name is "rf":
                    print lr.best_estimator_
                y_pred = lr.predict(X_test)
                # save results
                errors = np.abs(y_pred - y_test)
                e.data.append(errors)
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            p25 = lambda x: np.percentile(x, 25) * 100.
            p75 = lambda x: np.percentile(x, 75) * 100.
            m = lambda x: np.median(x) * 100.
            plt.fill_between(np.array(noise_levels) * 100.,
                             map(p25, e.data),
                             map(p75, e.data),
                             edgecolor=e.color, facecolor=e.color,
                             alpha=e.alpha)
            h, = plt.plot(np.array(noise_levels) * 100.,
                          map(m, e.data), color=e.color,
                          label=e.name)
            legend_handles.append(h)
        minor_ticks = np.arange(0, 80, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, 0.3), xytext=(10, 35), size=10, va="center",
                           ha="center", bbox=dict(boxstyle="round4",
                                                  fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)
        plt.title("dependency on wrong training noise [w_training = 10%]")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 80))
        plt.savefig(self.output().path + "_temp.png", dpi=500,
                    bbox_inches='tight')



class NoisePlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_no_aray_train"), \
                tasks_mc.CameraBatch("generic_tissue_no_aray_test")

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
        kf = sklearn.cross_validation.KFold(batch_train.reflectances.shape[0],
                                            10, shuffle=True)
        param_grid_rf = [
          {"n_estimators": np.array([50]),
           "max_depth": np.array([30]),  # np.arange(30, 35, 2),
           "min_samples_leaf": np.array([5])}]
        rf = sklearn.grid_search.GridSearchCV(RandomForestRegressor(50,
                                                    max_depth=10, n_jobs=-1),
                  param_grid_rf, cv=kf, n_jobs=-1)

        evaluation_setups = [
                    EvaluationStruct("classic", LinearSaO2Unmixing(), [],
                                     "red", 0.25),
                    EvaluationStruct("lasso", LassoCV(), [], "yellow", 0.5)
                    , EvaluationStruct("rf", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 1.02, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
                if e.name is "rf":
                    print lr.best_estimator_
                y_pred = lr.predict(X_test)
                # save results
                errors = np.abs(y_pred - y_test)
                e.data.append(errors)
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            p25 = lambda x: np.percentile(x, 25) * 100.
            p75 = lambda x: np.percentile(x, 75) * 100.
            m = lambda x: np.median(x) * 100.
            plt.fill_between(np.array(noise_levels) * 100.,
                             map(p25, e.data),
                             map(p75, e.data),
                             edgecolor=e.color, facecolor=e.color,
                             alpha=e.alpha)
            h, = plt.plot(np.array(noise_levels) * 100.,
                          map(m, e.data), color=e.color,
                          label=e.name)
            legend_handles.append(h)
        minor_ticks = np.arange(0, 80, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, 0.3), xytext=(10, 35), size=10, va="center",
                           ha="center", bbox=dict(boxstyle="round4",
                                                  fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)
        plt.title("dependency on noise")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 80))
        plt.savefig(self.output().path + "_temp.png", dpi=500,
                    bbox_inches='tight')


class DAPlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_no_aray_train"), \
                tasks_mc.CameraBatch("generic_tissue_no_aray_test"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_train"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_test")

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
        X_test, y_test = preprocess(batch_test, w_percent=w_standard)
        X_train, y_train = preprocess(batch_train, w_percent=w_standard)
        X_da_train, y_da_train = preprocess(batch_da_train,
                                                    w_percent=w_standard)
        X_da_test, y_da_test = preprocess(batch_da_test,
                                                  w_percent=w_standard)

#         transformer = PCA(n_components=10)
#         transformer = sklearn.preprocessing.StandardScaler()
#         transformer.fit(X_train)
#         X_train = transformer.fit_transform(X_train)
#         X_test = transformer.transform(X_test)
#         X_da_test = transformer.transform(X_da_test)
#         X_da_train = transformer.transform(X_da_train)

        # setup structures to save test data
        # train logistic regression
        kf = sklearn.cross_validation.KFold(X_train.shape[0], 10, shuffle=True)
        # todo include intercept scaling paramter
        param_grid_svm = [
          {'C': np.logspace(-6, 6, 1),
           'kernel':['rbf', 'linear'],
           'gamma': np.logspace(-6, 6, 1)} ]
        svm = sklearn.grid_search.GridSearchCV(sklearn.svm.SVR(),
                               param_grid_svm, cv=kf, n_jobs=-1)
        # use the random forest regressor
        param_grid_rf = [
          {"n_estimators": np.array([50]),
           "max_depth": np.arange(5, 30, 2),
           "min_samples_leaf": np.array([5, 7, 10, 15])}]
        rf = sklearn.grid_search.GridSearchCV(RandomForestRegressor(50,
                                                    max_depth=10, n_jobs=-1),
                  param_grid_rf, cv=kf, n_jobs=-1)
        # do not do cv for now to save some time.
        rf = RandomForestRegressor(50, max_depth=30, min_samples_leaf=5,
                                   n_jobs=-1)
        # svm = sklearn.svm.SVR(kernel='linear', C=0.0000001)

        evaluation_setups = [
                    EvaluationStruct("classic", LinearSaO2Unmixing(), [],
                                     "red", 0.5)
                    , EvaluationStruct("lr", LinearRegression(),
                                       [], "yellow", 0.5)
                    # , EvaluationStruct("svm weighting", svm, [], "blue", 0.5)
                    , EvaluationStruct("rf", rf, [], "green", 0.5)
                    , EvaluationStruct("rf no cov shift", rf, [], "blue", 0.5)
                    , EvaluationStruct("lr no cov shift", LinearRegression(),
                                       [], "blue", 0.5)
                    ]
        # estimate weights for domain adaptation
        lr = estimate_logistic_regressor(X_train, X_da_train)
        weights = lr.predict_proba(X_train)[:, 1] / lr.predict_proba(X_train)[:, 0]
        # X_train = X_train[:X_da_train.shape[0], :]
        # y_train = y_train[:X_da_train.shape[0]]

#         PE, weights, s = pyRULSIF.R_ULSIF(X_da_train.T, X_train.T, 0, 0.,
#                          pyRULSIF.sigma_list(X_da_train.T, X_train.T),
#                          pyRULSIF.lambda_list(), X_da_train.shape[0],
#                          5)
        # resample from X according da weighting result
#
#         batch_temp = copy.deepcopy(batch_train)
#         batch_temp2 = copy.deepcopy(batch_train)
#         batch_temp.reflectances = X_train[0:4, :]
#
#         normalizer = Normalizer(norm='l1')
#         reflectances = batch_train.reflectances
#         reflectances = normalizer.transform(reflectances)
#         batch_temp2.reflectances = -np.log(reflectances[0:4, :])
#         f, axarr = plt.subplots(2, 1)
#         from mc.plot import plot
#         plot(batch_temp, axarr[0])
#         plot(batch_temp2, axarr[1])
#         plt.show()

        X_resampled, y_resampled, weights = resample(X_train, y_train, weights)

        for e in evaluation_setups:
            # train
            if e.name is  "lasso+da":
                e.regressor.fit(X_resampled, y_resampled)
            elif e.name is "lr no cov shift" or e.name is "rf no cov shift":
                e.regressor.fit(X_da_train, y_da_train)
            elif e.name is "rf":
                e.regressor.fit(X_train, y_train)
                # print e.regressor.best_estimator_
            else:
                e.regressor.fit(X_train, y_train)
            # evaluate
            y_pred = e.regressor.predict(X_da_test)
            # save results
            e.data.append(np.abs(y_pred - y_da_test))

        # collect data for boxplot
        boxplot_data = []
        for e in evaluation_setups:
            boxplot_data.append(100. * e.data[0])
        # make a nice plot for results
        fig = plt.figure()
        ax = fig.add_subplot(111)
        bp = ax.boxplot(boxplot_data, showfliers=False,
                        showmeans=True, patch_artist=True)
        # nicen it up
        for e, box in zip(evaluation_setups, bp['boxes']):
            # change outline color
            box.set(color="black", linewidth=2)
            # change fill color
            box.set(facecolor=e.color, alpha=e.alpha)
        # change color and linewidth of the whiskers
        x_tick_labels = []
        for e, whisker, cap, median in itertools.izip(
                    evaluation_setups,
                    bp['whiskers'], bp['caps'], bp['medians']):
            whisker.set(color="black", linewidth=2)
            cap.set(color="black", linewidth=2)
            median.set(color="black", linewidth=2)
            x_tick_labels.append(e.name)
        ax.set_xticklabels(x_tick_labels)
        ax.get_xaxis().tick_bottom()
        ax.get_yaxis().tick_left()
        ax.axvline(x=3.5, linestyle="--", color="black", linewidth=2)
        plt.grid()
        plt.title("performance under covariance shift")
        plt.xlabel("method")
        plt.ylabel("absolute error [%]")
        # plt.ylim((0, 40))
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

