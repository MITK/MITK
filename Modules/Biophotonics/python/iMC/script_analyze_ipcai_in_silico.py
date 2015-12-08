# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import pickle
from collections import namedtuple
import luigi
import matplotlib.pyplot as plt
import matplotlib
from sklearn.ensemble.forest import RandomForestRegressor

import tasks_mc
import scriptpaths as sp
from regression.preprocessing import preprocess, preprocess2
from regression.linear import LinearSaO2Unmixing
from regression.domain_adaptation import *
from regression.estimation import standard_score


sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_in_silico"
sp.FINALS_FOLDER = "Images"
sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9
sp.MC_DATA_FOLDER = "mc_data2"

w_standard = 0.1  # for this evaluation we add 5% noise


matplotlib.rcParams.update({'font.size': 22})

EvaluationStruct = namedtuple("EvaluationStruct",
                              "name regressor data color alpha")


# setup standard random forest
rf = RandomForestRegressor(10, min_samples_leaf=10, max_depth=9, n_jobs=-1)

class TrainingSamplePlots(luigi.Task):

    which = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch("ipcai_" + self.which + "_train"), \
                tasks_mc.CameraBatch("ipcai_" + self.which + "_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "samples_plot_" + self.which + ".png"))
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
#                     EvaluationStruct("Linear Beer-Lambert",
#                                      LinearSaO2Unmixing(), [], "red", 0.5),
                    EvaluationStruct("Proposed", rf, "green", 0.5)]
        # do  validation
        nr_training_samples = np.arange(10, 15060, 50).astype(int)
        # not very pythonic, don't care
        for n in nr_training_samples:
            print "number samples ", str(n)
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
                          label=e.name)
            legend_handles.append(h)
        # minor_ticks_x = np.arange(0, 1000, 50)
        # minor_ticks_y = np.arange(5, 16, 0.5)
        # plt.gca().set_xticks(minor_ticks_x, minor=True)
        # plt.gca().set_yticks(minor_ticks_y, minor=True)
        plt.grid()
        # plt.legend(handles=legend_handles)
        # plt.title("dependency on number of training samples")
        plt.xlabel("number of training samples")
        plt.ylabel("absolute error [%]")

        for tick in plt.gca().xaxis.get_major_ticks():
            tick.label.set_fontsize(14)
        plt.ylim((2, 10))
        plt.xlim((0, 15000))
        plt.savefig(self.output().path,
                    dpi=500, bbox_inches='tight')


class WrongNoisePlots(luigi.Task):

    which = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch("ipcai_" + self.which + "_train"), \
                tasks_mc.CameraBatch("ipcai_" + self.which + "_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_wrong_plot_" + self.which + ".png"))


    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)

        evaluation_setups = [
                        EvaluationStruct("Linear Beer-Lambert",
                                         LinearSaO2Unmixing(), [], "red", 0.25)
                        , EvaluationStruct("Proposed", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 0.17, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=0.1)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
                y_pred = lr.predict(X_test)
                # save results
                errors = np.abs(y_pred - y_test)
                e.data.append(errors)
        # make a nice plot for results
        plt.figure()
        legend_handles = []
        median0 = 0
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
            if e.name is "Proposed":
                median0 = m(e.data[0])
        minor_ticks = np.arange(0, 15, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, median0), xytext=(5, 10), size=15,
                           va="center",
                           ha="center", bbox=dict(boxstyle="round4",
                                                  fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)
        # plt.title("dependency on wrong training noise [w_training = 10%]")
        plt.xlabel("noise added for testing [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 20))
        plt.xlim((0, 15))
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')



class NoisePlotsBVF(luigi.Task):

    which = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch("ipcai_" + self.which + "_train"), \
                tasks_mc.CameraBatch("ipcai_" + self.which + "_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_plot_bvf_" + self.which + ".png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)

        evaluation_setups = [
                    EvaluationStruct("Proposed", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 0.17, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess2(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess2(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
                y_pred = lr.predict(X_test)
                # save results
                errors = np.abs(y_pred[:, 0] - y_test[:, 0])
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
        minor_ticks = np.arange(0, 15, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)

        plt.grid()
        # plt.legend(handles=legend_handles)
        # plt.title("dependency on noise")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 4))
        plt.xlim((0, 15))
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')



class DomainNoisePlot(luigi.Task):

    def requires(self):
        return tasks_mc.CameraBatch("ipcai_colon_muscle_train"), \
                tasks_mc.CameraBatch("ipcai_generic_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_da_plot_.png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)

        evaluation_setups = [
                    EvaluationStruct("Linear Beer-Lambert",
                                     LinearSaO2Unmixing(), [], "red", 0.25)
                    , EvaluationStruct("Proposed", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 0.17, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
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
            if e.name is "Proposed":
                median0 = m(e.data[0])
        minor_ticks = np.arange(0, 15, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, median0), xytext=(5, 12), size=15,
                           va="center", ha="center", bbox=dict(
                                        boxstyle="round4", fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)

        # plt.title("dependency on noise")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 20))
        plt.xlim((0, 15))
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')



class NoisePlots(luigi.Task):

    which = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch("ipcai_" + self.which + "_train"), \
                tasks_mc.CameraBatch("ipcai_" + self.which + "_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "noise_plot_" + self.which + ".png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)

        evaluation_setups = [
                    EvaluationStruct("Linear Beer-Lambert",
                                     LinearSaO2Unmixing(), [], "red", 0.25)
                    , EvaluationStruct("Proposed", rf, [], "green", 0.5)
                    ]
        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 0.17, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                lr.fit(X_train, y_train)
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
            if e.name is "Proposed":
                median0 = m(e.data[0])
        minor_ticks = np.arange(0, 15, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, median0), xytext=(5, 10), size=15, va="center",
                           ha="center", bbox=dict(boxstyle="round4",
                                                  fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)
        # plt.title("dependency on noise")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 20))
        plt.xlim((0, 15))
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')



class WeightsCalculation(luigi.Task):
    source_domain = luigi.Parameter()
    target_domain = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.source_domain), \
               tasks_mc.CameraBatch(self.target_domain)

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
                                      magnification=None)
        X_da_train, y_da_train = preprocess(batch_da_train,
                                            w_percent=w_standard,
                                            magnification=None)
        X_ws, y_ws = preprocess(batch_train, w_percent=w_standard)
        weights = estimate_weights_random_forests(X_train, X_da_train, X_ws)


        no_zero = ~(batch_train.reflectances <= 0).any(axis=1)
        bvfs = batch_train.layers[0][no_zero, 0]
        saO2s = batch_train.layers[0][no_zero, 1]
        a_mies = batch_train.layers[0][no_zero, 2]
        ds = batch_train.layers[0][no_zero, 4]

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


class NewDAPlot(luigi.Task):
    def requires(self):
        return tasks_mc.CameraBatch("ipcai_generic"), \
                tasks_mc.CameraBatch("ipcai_colon_muscle_test"), \
                WeightsCalculation("ipcai_generic", "ipcai_colon_muscle_test")

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 sp.RESULTS_FOLDER,
                                 sp.FINALS_FOLDER,
                                 "new_da.png"))

    def run(self):
        # get data
        f = open(self.input()[0].path, "r")
        batch_train = pickle.load(f)
        f = open(self.input()[1].path, "r")
        batch_test = pickle.load(f)
        f = open(self.input()[2].path, "r")
        weights = np.load(f)

        evaluation_setups = [
                    EvaluationStruct("Linear Beer-Lambert",
                                     LinearSaO2Unmixing(), [], "red", 0.25)
                    , EvaluationStruct("Proposed", rf, [], "yellow", 0.5)
                    , EvaluationStruct("Proposed DA", rf, [], "green", 0.5)
                    ]

        X_train, y_train = preprocess(batch_train, w_percent=w_standard)
        X_da_test, y_da_test = preprocess(batch_test, w_percent=w_standard)
        X_train_sampled, y_train_sampled, weights_sampled = \
            resample(X_train, y_train, weights, 8000)
        f, axarr = plt.subplots(X_train.shape[1], 1)
        for i in range(X_train.shape[1]):
            y, binEdges = np.histogram(X_train[:, i], weights=weights)
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-+', label="adapted")

            y, binEdges = np.histogram(X_train[:, i])
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-o', label="original")

            y, binEdges = np.histogram(X_da_test[:, i],
                                       weights=np.ones(X_da_test.shape[0]) *
                                       X_train.shape[0] / X_da_test.shape[0])
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-*', label="searched")

            y, binEdges = np.histogram(X_train_sampled[:, i],
                                    weights=np.ones(X_train_sampled.shape[0]) *
                                    X_train.shape[0] / X_train_sampled.shape[0])
            bincenters = 0.5 * (binEdges[1:] + binEdges[:-1])
            axarr[i].plot(bincenters, y, '-^', label="sampled")

            axarr[i].xaxis.set_visible(False)
            axarr[i].yaxis.set_visible(False)
            axarr[i].set_title("feature " + str(i) + " distribution")

        plt.legend(loc="best")
        # plt.tight_layout(h_pad=.5)
        plt.show()

        # iterate over different levels of noise
        noise_levels = np.arange(0.00, 0.17, 0.02)
        for w in noise_levels:
            # setup testing function
            X_test, y_test = preprocess(batch_test, w_percent=w)
            # extract noisy data
            X_train, y_train = preprocess(batch_train, w_percent=w)
            for e in evaluation_setups:
                lr = e.regressor
                if "DA" in e.name:
                    lr.fit(X_train, y_train, weights)
                else:
                    lr.fit(X_train, y_train)
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
            if e.name is "Proposed":
                median0 = m(e.data[0])
        minor_ticks = np.arange(0, 15, 5)
        plt.gca().set_xticks(minor_ticks, minor=True)
        plt.gca().set_yticks(minor_ticks, minor=True)

        plt.gca().annotate('error: ' +
                           "{0:.2f}".format(
                                        np.median(evaluation_setups[-1].data[0])
                                        * 100) + "%",
                           xy=(0, median0), xytext=(5, 10), size=15, va="center",
                           ha="center", bbox=dict(boxstyle="round4",
                                                  fc="g", alpha=0.5),
                           arrowprops=dict(arrowstyle="-|>",
                                  connectionstyle="arc3,rad=-0.2",
                                  fc="w"),
                    )
        plt.grid()
        plt.legend(handles=legend_handles)

        # plt.title("dependency on noise")
        plt.xlabel("noise added [sigma %]")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 20))
        plt.xlim((0, 15))
        plt.savefig(self.output().path + "temp.png", dpi=500,
                    bbox_inches='tight')




if __name__ == '__main__':
    # root folder there the data lies
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    w.add(WrongNoisePlots(which="colon_muscle"))
    # w.add(WrongNoisePlots(which="generic"))
    w.add(TrainingSamplePlots(which="colon_muscle"))
    w.add(TrainingSamplePlots(which="generic"))
    w.add(NoisePlots(which="colon_muscle"))
    w.add(DomainNoisePlot())
    w.add(NoisePlotsBVF(which="colon_muscle"))
    # w.add(NoisePlots(which="generic"))
    w.add(NewDAPlot())
    w.run()

