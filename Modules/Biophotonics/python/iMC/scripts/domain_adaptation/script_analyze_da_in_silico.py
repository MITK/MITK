# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""

import os
import logging
import datetime

import numpy as np
import pandas as pd
import luigi
import matplotlib.pyplot as plt
import seaborn as sns

from regression.preprocessing import preprocess
import domain_adaptation_paths as sp
from ipcai2016 import tasks_mc
from ipcai2016.script_analyze_ipcai_in_silico import w_standard, noise_levels, \
    evaluate_data, standard_plotting, NoisePlot
# additionally we need the weights estimation functionality
from regression.domain_adaptation import estimate_weights_random_forests


class WeightedBatch(luigi.Task):
    which_source = luigi.Parameter()
    which_target = luigi.Parameter()
    noise = luigi.FloatParameter()

    def requires(self):
        return tasks_mc.CameraBatch(self.which_source), \
               tasks_mc.CameraBatch(self.which_target)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              "adapted_" +
                                              self.which_source +
                                              "_with_" + self.which_target +
                                              "_noise_" + str(self.noise) +
                                              ".txt"))

    def run(self):
        # get data
        df_source = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_target = pd.read_csv(self.input()[1].path, header=[0, 1])

        # first extract X_source and X_target, preprocessed at standard noise
        # level
        X_source, y_source = preprocess(df_source, w_percent=w_standard)
        X_target, y_target = preprocess(df_target, w_percent=w_standard)

        # train a classifier to determine probability for specific class
        weights = estimate_weights_random_forests(X_source, X_target, X_source)
        # add weight to dataframe
        df_source["weights"] = weights

        # finally save the dataframe with the added weights
        df_source.to_csv(self.output().path, index=False)


class DaNoisePlot(luigi.Task):
    """
    Very similar to NoisePlot in IPCAI in silico evaluation but with
    weighted data coming in.
    """
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()

    def requires(self):
        # for each noise level we need to create the weights
        NecessaryBatches = map(lambda noise: WeightedBatch(self.which_train,
                                                           self.which_test,
                                                           noise),
                               noise_levels)
        return NecessaryBatches(self.which_train, self.which_test), \
               tasks_mc.CameraBatch(self.which_test)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "noise_da_plot_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".png"))

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])

        df = evaluate_data(df_train, noise_levels, df_test, noise_levels)
        standard_plotting(df)

        # finally save the figure
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')


class GeneratingDistributionPlot(luigi.Task):
    which_source = luigi.Parameter()
    which_target = luigi.Parameter()

    def requires(self):
        return WeightedBatch(self.which_source, self.which_target), \
               tasks_mc.CameraBatch(self.which_target)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "generating_distribution_" +
                                              self.which_source +
                                              "_adapted_to_" +
                                              self.which_target +
                                              ".png"))

    def run(self):
        # get data
        df_source = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_target = pd.read_csv(self.input()[1].path, header=[0, 1])

        # create dataframe suited for plotting
        # we do a weighted sampling with replacement to be able to create some
        # plots there the data distribution is visible.
        nr_samples = 100
        # first data weighted by domain adaptation
        df_source_adapted = df_source["layer0"].copy()
        df_source_adapted["weights"] = df_source["weights"]
        df_source_adapted["data"] = "adapted"
        df_source_adapted = df_source_adapted.sample(n=nr_samples, replace=True,
                                                     weights="weights")
        # now original source data
        df_source = df_source["layer0"].copy()
        df_source["weights"] = 1 # we set the weights here to 1
        df_source["data"] = "source"
        df_source = df_source.sample(n=nr_samples, replace=True,
                                     weights="weights")
        # now the target data
        df_target = df_target["layer0"]
        df_target["weights"] = 1
        df_target["data"] = "target"
        df_target = df_target.sample(n=nr_samples, replace=True,
                                     weights="weights")
        # now merge all three dataframes to the dataframe used for the plotting
        df = pd.concat([df_source, df_source_adapted, df_target])
        # since we already sampled we can get rid of weights
        df.drop("weights", axis=1, inplace=True)
        # d to um
        df["d"] *= 10**6
        # vhb and sao2 to %
        df["vhb"] *= 100
        df["sao2"] *= 100

        # do some serious plotting
        g = sns.pairplot(df, vars=["vhb", "sao2", "d"],
                         hue="data", markers=["o", "s", "D"])

        # tidy up plot
        g.add_legend()

        # finally save the figure
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')


class WeightDistributionPlot(luigi.Task):
    which_source = luigi.Parameter()
    which_target = luigi.Parameter()

    def requires(self):
        return WeightedBatch(self.which_source, self.which_target)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "weight_distribution_" +
                                              self.which_source +
                                              "_adapted_to_" +
                                              self.which_target +
                                              ".png"))

    def run(self):
        # get data
        df_source = pd.read_csv(self.input().path, header=[0, 1])

        df_source["weights"].plot.hist(bins=100)
        plt.axvline(x=1, ymin=0, ymax=df_source.shape[0])
        # TODO: add cumsum on top

        # finally save the figure
        plt.savefig(self.output().path, dpi=500,
                    bbox_inches='tight')


class FeatureDistributionPlot(luigi.Task):
    which_source = luigi.Parameter()
    which_target = luigi.Parameter()

    def requires(self):
        return WeightedBatch(self.which_source, self.which_target), \
               tasks_mc.CameraBatch(self.which_target)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "feature_distribution_" +
                                              self.which_source +
                                              "_adapted_to_" +
                                              self.which_target +
                                              ".png"))

    def run(self):
        # get data
        df_source = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_target = pd.read_csv(self.input()[1].path, header=[0, 1])

        df_f_source = format_dataframe_for_distribution_plotting(df_source)
        df_f_target = format_dataframe_for_distribution_plotting(df_target)
        df_f_adapted = format_dataframe_for_distribution_plotting(df_source,
                                weights=df_source["weights"].values.squeeze())

        # build a combined df
        df_f_source["data"] = "source"
        df_f_target["data"] = "target"
        df_f_adapted["data"] = "adapted"
        df = pd.concat([df_f_source, df_f_target, df_f_adapted])

        # do the plotting
        grid = sns.FacetGrid(df, col="w", hue="data", col_wrap=3, size=1.5)
        grid.map(plt.plot, "bins", "frequency")

        # tidy up plot
        grid.fig.tight_layout(w_pad=1)
        grid.add_legend()
        grid.set(xticks=(0, 1))

        # finally save the figure
        plt.savefig(self.output().path, dpi=500)


class DAvNormalPlot(luigi.Task):
    which_train = luigi.Parameter()
    which_test = luigi.Parameter()
    which_train_no_covariance_shift = luigi.Parameter()

    def requires(self):
        return WeightedBatch(self.which_train, self.which_test), \
               tasks_mc.CameraBatch(self.which_test), \
               tasks_mc.CameraBatch(self.which_train_no_covariance_shift)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "da_v_normal_train_" +
                                              self.which_train +
                                              "_test_" + self.which_test +
                                              ".png"))

    def run(self):
        # get data
        df_train = pd.read_csv(self.input()[0].path, header=[0, 1])
        df_test = pd.read_csv(self.input()[1].path, header=[0, 1])
        df_train_no_covariance_shift = pd.read_csv(self.input()[2].path,
                                                   header=[0, 1])

        evaluation_setups = [EvaluationStruct("Proposed", rf)]
        # evaluate the different methods
        df_adapted = evaluate_data(df_train, noise_levels,
                                   df_test, noise_levels,
                                   evaluation_setups=evaluation_setups)
        df_adapted["data"] = "adapted"
        df_no_adaptation = evaluate_data(
                df_train.drop("weights", axis=1), noise_levels,
                df_test, noise_levels,
                evaluation_setups=evaluation_setups)
        df_no_adaptation["data"] = "source"
        df_no_covariance_shift = evaluate_data(
                df_train_no_covariance_shift, noise_levels,
                df_test, noise_levels,
                evaluation_setups=evaluation_setups)
        df_no_covariance_shift["data"] = "target"
        df = pd.concat([df_adapted, df_no_adaptation, df_no_covariance_shift])

        # plot it
        sns.boxplot(data=df, x="noise added [sigma %]", y="Errors", hue="data",
                    hue_order=["source", "adapted", "target"], fliersize=0)
        # tidy up plot
        plt.ylim((0, 40))
        plt.legend(loc='upper left')

        # finally save the figure
        plt.savefig(self.output().path, dpi=500)


def format_dataframe_for_distribution_plotting(df, weights=None):
    if weights is None:
        weights = np.ones(df.shape[0])

    bins = np.arange(0, 1, 0.01)

    # we're only interested in reflectance information
    df_formatted = df.loc[:, "reflectances"]
    # to [nm] for plotting
    df_formatted.rename(columns=lambda x: float(x)*10**9, inplace=True)

    # transform data to a histogram
    df_formatted = df_formatted.apply(lambda x:
                                      pd.Series(np.histogram(x, bins=bins,
                                                             weights=weights,
                                                             normed=True)[0]),
                                      axis=0)
    # convert to long form using bins as identifier
    df_formatted["bins"] = bins[1:]
    df_formatted = pd.melt(df_formatted, id_vars=["bins"],
                           var_name="w", value_name="frequency")

    return df_formatted


if __name__ == '__main__':
    logging.basicConfig(filename=os.path.join(sp.LOG_FOLDER,
                                              "da_in_silico_plots" +
                                              str(datetime.datetime.now()) +
                                              '.log'),
                        level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)
    luigi.interface.setup_interface_logging()

    source_domain = "ipcai_revision_generic"
    target_domain = "ipcai_revision_colon_test"

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    # check how the graph looks with same domains for training and testing
    w.add(DaNoisePlot(which_train="ipcai_revision_colon_train",
                      which_test="ipcai_revision_colon_test"))
    # check how the graph looks without domain adaptation
    w.add(NoisePlot(which_train=source_domain, which_test=target_domain))
    # Set a different testing domain to evaluate domain sensitivity
    w.add(DaNoisePlot(which_train=source_domain, which_test=target_domain))

    w.add(WeightDistributionPlot(which_source=source_domain,
                                 which_target=target_domain))
    w.add(FeatureDistributionPlot(which_source=source_domain,
                                  which_target=target_domain))
    # also plot distributions for equal domains to check for errors in data
    w.add(FeatureDistributionPlot(which_source="ipcai_revision_colon_mean_scattering_train",
                                  which_target="ipcai_revision_colon_mean_scattering_test"))
    # plot how the generating model data (e.g. sao2 and vhb) is distributed
    w.add(GeneratingDistributionPlot(which_source=source_domain,
                                     which_target=target_domain))
    w.add(DAvNormalPlot(which_train=source_domain, which_test=target_domain,
                which_train_no_covariance_shift="ipcai_revision_colon_train"))
    w.run()
