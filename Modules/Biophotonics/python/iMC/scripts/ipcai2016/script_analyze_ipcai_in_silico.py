# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import logging
import datetime
from collections import namedtuple

import matplotlib
import numpy as np
import pandas as pd
from pandas import DataFrame
import luigi
import matplotlib.pyplot as plt
from sklearn.ensemble.forest import RandomForestRegressor

import tasks_mc
from regression.preprocessing import preprocess, preprocess2
from regression.linear import LinearSaO2Unmixing

import commons

sc = commons.ScriptCommons()

sc.add_dir("IN_SILICO_RESULTS_PATH", os.path.join(sc.get_dir("RESULTS_FOLDER"),
                                     "in_silico"))

sc.other["RECORDED_WAVELENGTHS"] = np.arange(470, 680, 10) * 10 ** -9

w_standard = 10.  # for this evaluation we add 10% noise

font = {'family' : 'normal',
        'size'   : 20}

matplotlib.rc('font', **font)


# setup standard random forest
rf = RandomForestRegressor(10, min_samples_leaf=10, max_depth=9, n_jobs=-1)
EvaluationStruct = namedtuple("EvaluationStruct",
                              "name regressor")
# standard evaluation setup
standard_evaluation_setups = [EvaluationStruct("Linear Beer-Lambert",
                                               LinearSaO2Unmixing())
                              , EvaluationStruct("Proposed", rf)]

# color palette
my_colors = ["red", "green"]

# standard noise levels
noise_levels = np.array([1,2,3,4,5,6,7,8,9,10,
                         15,20,30,40,50,100,150,200]).astype("float")


class TrainingSamplePlot(luigi.Task):
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.which_train), \
               tasks_mc.CameraBatch(self.which_test)

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("IN_SILICO_RESULTS_PATH"),
                                              "sample_plot_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".pdf"))

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])

        # for this plot we write a custom evaluation function as it is built
        # a little different

        # create a new dataframe which will hold all the generated errors
        df = pd.DataFrame()

        nr_training_samples = np.arange(10, 15010, 50).astype(int)
        # not very pythonic, don't care
        for n in nr_training_samples:
            X_test, y_test = preprocess(df_test, snr=w_standard)
            # only take n samples for training
            X_train, y_train = preprocess(df_train, nr_samples=n,
                                          snr=w_standard)

            regressor = rf
            regressor.fit(X_train, y_train)
            y_pred = regressor.predict(X_test)
            # save results to a dataframe
            errors = np.abs(y_pred - y_test)
            errors = errors.reshape(len(errors), 1)
            current_df = DataFrame(errors * 100,
                                   columns=["Errors"])
            current_df["Method"] = "Proposed"
            current_df["Number Samples"] = n / 10**3.
            df = pd.concat([df, current_df], ignore_index=True)
            logging.info(
                    "Finished training classifier with {0} samples".format(
                            str(n)))

        df = df.groupby("Number Samples").describe()
        # get the error description in the rows:
        df = df.unstack(-1)
        # get rid of multiindex by dropping "Error" level
        df.columns = df.columns.droplevel(0)

        plt.figure()
        plt.plot(df.index, df["50%"], color="green")

        # tidy up the plot
        plt.xlabel("number of training samples / 1000")
        plt.ylabel("absolute error [%]")
        plt.ylim((0, 20))
        plt.xlim((0, 15))
        plt.grid()

        # finally save the figure
        plt.savefig(self.output().path, mode="pdf", dpi=500,
                    bbox_inches='tight')


class VhbPlot(luigi.Task):
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.which_train), \
               tasks_mc.CameraBatch(self.which_test)

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("IN_SILICO_RESULTS_PATH"),
                                              "vhb_noise_plot_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".pdf"))

    @staticmethod
    def preprocess_vhb(batch, nr_samples=None, snr=None,
                       magnification=None, bands_to_sortout=None):
        """ For evaluating vhb we extract labels for vhb instead of sao2"""
        X, y = preprocess2(batch, nr_samples, snr,
                           magnification, bands_to_sortout)

        return X, y["vhb"].values

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])

        # for vhb we only evaluate the proposed method since the linear
        # beer-lambert is not applicable
        evaluation_setups = [EvaluationStruct("Proposed", rf)]
        df = evaluate_data(df_train, noise_levels, df_test, noise_levels,
                           evaluation_setups=evaluation_setups,
                           preprocessing=self.preprocess_vhb)
        standard_plotting(df, color_palette=["green"],
                          xytext_position=(2, 3))
        plt.ylim((0, 4))

        # finally save the figure
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')


class NoisePlot(luigi.Task):
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.which_train), \
               tasks_mc.CameraBatch(self.which_test)

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("IN_SILICO_RESULTS_PATH"),
                                              "noise_plot_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".pdf"))

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])

        df = evaluate_data(df_train, noise_levels, df_test, noise_levels)
        standard_plotting(df)

        # finally save the figure
        plt.savefig(self.output().path, mode="pdf", dpi=500,
                    bbox_inches='tight')


class WrongNoisePlot(luigi.Task):
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()
    train_snr = luigi.FloatParameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.which_train), \
               tasks_mc.CameraBatch(self.which_test)

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("IN_SILICO_RESULTS_PATH"),
                                              str(self.train_snr) +
                                              "_wrong_noise_plot_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".pdf"))

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])

        # do same as in NoisePlot but with standard noise input
        df = evaluate_data(df_train,
                           np.ones_like(noise_levels) * self.train_snr,
                           df_test, noise_levels)
        standard_plotting(df)

        # finally save the figure
        plt.savefig(self.output().path, mode="pdf", dpi=500,
                    bbox_inches='tight')


def evaluate_data(df_train, w_train, df_test, w_test,
                  evaluation_setups=None, preprocessing=None):
    """ Our standard method to evaluate the data. It will fill a DataFrame df
    which saves the errors for each evaluated setup"""
    if evaluation_setups is None:
        evaluation_setups = standard_evaluation_setups
    if preprocessing is None:
        preprocessing = preprocess
    if ("weights" in df_train) and df_train["weights"].size > 0:
        weights = df_train["weights"].as_matrix().squeeze()
    else:
        weights = np.ones(df_train.shape[0])

    # create a new dataframe which will hold all the generated errors
    df = pd.DataFrame()
    for one_w_train, one_w_test in zip(w_train, w_test):
        # setup testing function
        X_test, y_test = preprocessing(df_test, snr=one_w_test)
        # extract noisy data
        X_train, y_train = preprocessing(df_train, snr=one_w_train)
        for e in evaluation_setups:
            regressor = e.regressor
            regressor.fit(X_train, y_train, weights)
            y_pred = regressor.predict(X_test)
            # save results to a dataframe
            errors = np.abs(y_pred - y_test)
            errors = errors.reshape(len(errors), 1)
            current_df = DataFrame(errors * 100,
                                   columns=["Errors"])
            current_df["Method"] = e.name
            current_df["SNR"] = int(one_w_test)
            df = pd.concat([df, current_df], ignore_index=True)

    return df


def standard_plotting(df, color_palette=None, xytext_position=None):
    if color_palette is None:
        color_palette = my_colors
    if xytext_position is None:
        xytext_position = (2, 15)

    plt.figure()

    # group it by method and noise level and get description on the errors
    df_statistics = df.groupby(['Method', 'SNR']).describe()
    # get the error description in the rows:
    df_statistics = df_statistics.unstack(-1)
    # get rid of multiindex by dropping "Error" level
    df_statistics.columns = df_statistics.columns.droplevel(0)

    # iterate over methods to plot linegraphs with error tube
    # probably this can be done nicer, but no idea how exactly

    for color, method in zip(
            color_palette, df_statistics.index.get_level_values("Method").unique()):
        df_method = df_statistics.loc[method]
        plt.plot(df_method.index, df_method["50%"],
                 color=color, label=method)
        plt.fill_between(df_method.index, df_method["25%"], df_method["75%"],
                         facecolor=color, edgecolor=color,
                         alpha=0.5)
    # tidy up the plot
    plt.ylim((0, 40))
    plt.gca().set_xticks(np.arange(0, 200, 10), minor=True)
    plt.xlabel("SNR")
    plt.ylabel("absolute error [%]")
    plt.grid()
    plt.legend()


if __name__ == '__main__':

    sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
    sc.create_folders()

    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                              "in_silico_plots_" +
                                              str(datetime.datetime.now()) +
                                              '.log'),
                        level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)
    luigi.interface.setup_interface_logging()

    train = "ipcai_revision_colon_mean_scattering_train"
    test = "ipcai_revision_colon_mean_scattering_test"

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    w.add(TrainingSamplePlot(which_train=train, which_test=test))
    w.add(NoisePlot(which_train=train, which_test=test))
    w.add(WrongNoisePlot(which_train=train, which_test=test, train_snr=10.))
    w.add(WrongNoisePlot(which_train=train, which_test=test, train_snr=50.))
    w.add(WrongNoisePlot(which_train=train, which_test=test, train_snr=200.))
    # Set a different testing domain to evaluate domain sensitivity
    w.add(NoisePlot(which_train=train,
                    which_test="ipcai_revision_generic_mean_scattering_test"))
    w.add(VhbPlot(which_train=train, which_test=test))
    w.run()
