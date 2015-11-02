# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import pickle
from collections import namedtuple
import itertools
import time

import luigi
import matplotlib.pyplot as plt
from sklearn.neighbors import KNeighborsRegressor
from sklearn.ensemble.forest import RandomForestRegressor
import sklearn.svm
import sklearn.grid_search
import sklearn.cross_validation
from sklearn.linear_model import *
from sklearn.decomposition import PCA
import scipy.io

import tasks_mc
import scriptpaths as sp
from regression.preprocessing import preprocess
from regression.linear import LinearSaO2Unmixing
from regression.domain_adaptation import *
from regression.estimation import standard_score


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
        return tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_train"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_test")

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
                                     [], "red", 0.5),
                    EvaluationStruct("random forest", RandomForestRegressor(50, max_depth=30, min_samples_leaf=5,
                                   n_jobs=-1), [], "green", 0.5)]
        # do  validation
        nr_training_samples = np.arange(10, 11000, 50).astype(int)
        # not very pythonic, don't care
        for n in nr_training_samples:
            print "nr samples ", str(n)
            for e in evaluation_setups:
                X_j = X_train[0:n]
                y_j = y_train[0:n]
                lr = e.regressor
                lr.fit(X_j, y_j)
                # save results
                e.data.append(standard_score(lr, X_test, y_test) * 100.)
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        for e in evaluation_setups:
            h, = plt.plot(nr_training_samples,
                          e.data, color=e.color,
                          label=e.regressor.__class__.__name__)
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



class WeightsCalculation(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_no_aray_train"), \
               tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_train")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "weights.npy"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_da_train = pickle.load(f)
        X_train, y_train = preprocess(batch_train, w_percent=w_standard,
                                      magnification=10)
        X_da_train, y_da_train = preprocess(batch_da_train,
                                            w_percent=w_standard, magnification=10)
        X_ws, y_ws = preprocess(batch_train, w_percent=w_standard)
        weights = estimate_weights_random_forests(X_train, X_da_train, X_ws)

        bvfs = batch_train.layers[0][:, 0]
        saO2s = batch_train.layers[0][:, 1]
        a_mies = batch_train.layers[0][:, 2]
        ds = batch_train.layers[0][:, 4]

        correct_high_weights = np.logical_and(bvfs < 0.10, ds < 1000 * 10 ** -6, ds > 600 * 10 ** -6)

        plt.figure()
        plt.hist(bvfs, weights=weights, alpha=0.5, label="found_bvfs")
        plt.hist(bvfs[correct_high_weights], alpha=0.5, label="true_bvfs")
        plt.title("weight distr bvf")
        plt.legend(loc='upper right')
        plt.figure()
        plt.hist(saO2s, weights=weights, alpha=0.5, label="found_saO2s")
        plt.hist(saO2s[correct_high_weights], alpha=0.5, label="true_saO2s")
        plt.title("weight distr saO2")
        plt.legend(loc='upper right')
        plt.figure()
        plt.hist(a_mies, weights=weights, alpha=0.5, label="found_a_mies")
        plt.hist(a_mies[correct_high_weights], alpha=0.5, label="true_a_mies")
        plt.title("weight distr a_mie")
        plt.legend(loc='upper right')
        plt.figure()
        plt.hist(ds, weights=weights, alpha=0.5, label="found_ds")
        plt.hist(ds[correct_high_weights], alpha=0.5, label="true_ds")
        plt.title("weight distr d")
        plt.legend(loc='upper right')
        plt.show()

        np.save(self.output().path, weights)


class DAPlots(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("generic_tissue_no_aray_train"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_train"), \
                tasks_mc.CameraBatch("colon_muscle_tissue_d_ranges_train"), \
                WeightsCalculation()

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
        batch_da_train = pickle.load(f)
        f = open(self.input()[2].path, "r")
        batch_da_test = pickle.load(f)
        weights = np.load(self.input()[3].path)
        X_train, y_train = preprocess(batch_train, w_percent=w_standard)
        X_da_train, y_da_train = preprocess(batch_da_train,
                                                    w_percent=w_standard)
        X_da_test, y_da_test = preprocess(batch_da_test,
                                                  w_percent=w_standard)
        scipy.io.savemat('/home/wirkert/workspace/matlab/domain_adaptation/GMKLIEP_demo/x_nu', mdict={'arr': X_da_train})
        scipy.io.savemat('/home/wirkert/workspace/matlab/domain_adaptation/GMKLIEP_demo/x_de', mdict={'arr': X_train})
        scipy.io.savemat('/home/wirkert/workspace/matlab/domain_adaptation/GMKLIEP_demo/bvf_de', mdict={'arr': batch_train.layers[0][:, 0]})
        # setup structures to save test data
        kf = sklearn.cross_validation.KFold(X_train.shape[0], 10, shuffle=True)
        param_grid_svm = [
          {'C': np.logspace(1, 8, 8),
           'kernel':['rbf'],
           'gamma': np.logspace(-3, 3, 7),
           'nu':np.array([0.2, 0.5, 0.7])} ]
        svm = sklearn.grid_search.GridSearchCV(sklearn.svm.NuSVR(),
                               param_grid_svm, cv=kf, n_jobs=-1)
        svm = sklearn.svm.SVR(kernel="rbf", degree=3, C=100., gamma=10.)
        # use the random forest regressor
        param_grid_rf = [
          {"n_estimators": np.array([50]),
           "max_depth": np.array([30]),  # np.arange(5, 40, 2),
           "max_features": np.array([8]),  # np.array(range(X_train.shape[1] - 1)) + 1,
           "min_weight_fraction_leaf": np.array([0.01, 0.03, 0.05, 0.07, 0.1, 0.2])}]  # , 7, 10, 15])}]
        rf = sklearn.grid_search.GridSearchCV(RandomForestRegressor(50,
                                                    max_depth=15, n_jobs=-1),
                  param_grid_rf, cv=kf, n_jobs=-1)
        # do not do cv for now to save some time.
        rf = RandomForestRegressor(20, max_depth=10,  # min_weight_fraction_leaf=0.05,
                                    n_jobs=-1)


        evaluation_setups = [
                    EvaluationStruct("classic", LinearSaO2Unmixing(), [],
                                     "red", 0.5)
                    , EvaluationStruct("lr", LinearRegression(), [],
                                       "yellow", 0.5)
                    , EvaluationStruct("knn",
                                        KNeighborsRegressor(),
                                        [], "yellow", 0.5)
                    , EvaluationStruct("rf", rf, [], "green", 0.5)
                    , EvaluationStruct("svr", svm, [], "yellow", 0.5)
#                    , EvaluationStruct("weighted rf", rf, [], "green", 0.5)
                    , EvaluationStruct("weighted svm", svm, [], "yellow", 0.5)
                     , EvaluationStruct("svm no cov shift", svm, [], "blue", 0.5)
                    ]


        bvfs = batch_train.layers[0][:, 0]
        saO2s = batch_train.layers[0][:, 1]
        a_mies = batch_train.layers[0][:, 2]
        ds = batch_train.layers[0][:, -1]

#         weights = scipy.io.loadmat("/media/wirkert/data/Data/2015_11_12_IPCAI_in_silico/weights_LHSS.mat")
#         weights = weights['r']
#         weights = scipy.io.loadmat("/media/wirkert/data/Data/2015_11_12_IPCAI_in_silico/weights.mat")
#         weights = weights['wh_x_de']
#         weights = np.squeeze(weights)

        f, axarr = plt.subplots(X_train.shape[1], 1)

#         X_train, y_train, weights = resample(X_train, y_train, weights, 5000)
#         pca = PCA(copy=True, n_components=5, whiten=True)
#         pca.fit(X_train)
#         X_train = pca.transform(X_train)
#         X_da_train = pca.transform(X_da_train)
#         X_da_test = pca.transform(X_da_test)
        for i in range(X_train.shape[1]):
            y, binEdges = np.histogram(X_train[:, i], weights=weights)
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-+', label="adapted")

            y, binEdges = np.histogram(X_train[:, i])
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-o', label="original")

            y, binEdges = np.histogram(X_da_train[:, i])
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-*', label="searched")

            axarr[i].set_title("feature " + str(i) + " distribution")
        plt.legend(loc="best")
        # plt.tight_layout(h_pad=.5)
#        plt.show()

#         for i in range(10):
#             relevant_elements = np.logical_and(bvfs > 0.01 * i,
#                                                bvfs < 0.01 * (i + 1))
#             relevant_saO2s = saO2s[relevant_elements]
#             relevant_weights = weights[relevant_elements]
#             plt.figure()
#             plt.hist(relevant_saO2s)  # , weights=relevant_weights)
#             plt.savefig(self.output().path + "saO2_distribution_" + str(i) +
#                         ".png")


        for e in evaluation_setups:
            # train
            start = time.time()
            if "no cov shift" in e.name:
                e.regressor.fit(X_da_train[0:-1], y_da_train[0:-1])
            elif "weighted" in e.name:
                # print "number of used samples", np.sum(weights > 0.4)
                # rw = weights > 1.
                # e.regressor.fit(X_train[rw, :], y_train[rw])
                e.regressor.fit(X_train, y_train, weights)
                # tree.export_graphviz(rf.estimators_[0], out_file='tree.dot')
            else:
                e.regressor.fit(X_train, y_train)
            # print some interesting statistics
            end = time.time()
            print "time to train", e.name, ":", str(end - start), "s"
            if isinstance(e.regressor, sklearn.grid_search.GridSearchCV):
                print e.regressor.best_estimator_

            # evaluate
            start = time.time()
            y_pred = e.regressor.predict(X_da_test)
            end = time.time()
            print "time to evaluate ", str(X_da_test.shape[0]), "samples: ", \
                str(end - start), "s"

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
        ax.axvline(x=len(evaluation_setups) - 1.5,
                   linestyle="--", color="black", linewidth=2)
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

