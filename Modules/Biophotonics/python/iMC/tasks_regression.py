'''
Created on Aug 26, 2015

@author: wirkert
'''


import os
import glob
import pickle
import numpy as np

from sklearn.ensemble import RandomForestRegressor
from sklearn.grid_search import GridSearchCV
from sklearn.cross_validation import KFold
from sklearn import tree
from sklearn.linear_model import LogisticRegression
import luigi

from msi.normalize import standard_normalizer
from msi.msi import Msi
from msi.imgmani import select_n_reflectances
from msi.io.nrrdreader import NrrdReader
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import tasks_preprocessing as ppt
import scriptpaths as sp


def extract_mc_data(parameters_file, reflectances_file):
    params = np.load(parameters_file)
    refls = np.load(reflectances_file)
    # create fake msi to be able to use normalizer
    fake_msi = Msi(image=refls)
    # if normalizer which requires wavelengths is needed these have to be
    # added here
    standard_normalizer.normalize(fake_msi)
    reflectances = fake_msi.get_image()
    # scaled_reflectances = preprocessing.scale(reflectances)
    # return only the first four parameters, as these indicate
    # BVF, DSP, d and SaO2. The others are noise parameters as e.g. submucosa
    # BVF and submucosa DSP.
    return params, reflectances
    # use this line as a hack if forward model should be evaluated:
    # return reflectances, params


def calc_weights(sourceReflectances, targetReflectances):
    targetReflectances = select_n_reflectances(targetReflectances,
                                               sourceReflectances.shape[0])
    labelsSource = np.zeros(sourceReflectances.shape[0])
    labelsTarget = np.ones(targetReflectances.shape[0])
    assert(labelsSource.shape == labelsTarget.shape)
    allReflectances = np.concatenate((sourceReflectances, targetReflectances))
    allLabels = np.concatenate((labelsSource, labelsTarget))

    # train logistic regression
    kf = KFold(allReflectances.shape[0], 5, shuffle=True)
    # todo include intercept scaling paramter
    param_grid = [
      {'C': np.logspace(-3, 6, 10), 'fit_intercept':['True', 'False']} ]
    best_lr = GridSearchCV(LogisticRegression(), param_grid, cv=kf, n_jobs=-1)
    best_lr.fit(allReflectances, allLabels)
    sourceProbabilities = best_lr.predict_proba(sourceReflectances)

    return sourceProbabilities[:, 1] / sourceProbabilities[:, 0]


class ParameterFile(luigi.Task):
    file_name_prefix = luigi.Parameter()

    def output(self):
        filename = glob.glob(os.path.join(sp.ROOT_FOLDER,
                                          sp.MC_DATA_FOLDER,
                                          self.file_name_prefix + "_*"))
        return luigi.LocalTarget(os.path.abspath(filename[0]))


class ReflectanceFile(luigi.Task):
    file_name_prefix = luigi.Parameter()

    def output(self):
        filename = glob.glob(os.path.join(sp.ROOT_FOLDER,
                                          sp.MC_DATA_FOLDER,
                                          self.file_name_prefix +
                                          "reflectances*"))
        return luigi.LocalTarget(os.path.abspath(filename[0]))


class RegressorFile(luigi.Task):
    regressor = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                 self.regressor))


def train(x, y):
    # set up cross-validation and grid search
    kf = KFold(x.shape[0], 5, shuffle=True)
    param_grid = [
      {"n_estimators": np.array([50]),
       "max_depth": np.arange(5, 15, 2),
       "min_samples_leaf": np.array([1, 10, 15, 20]) * x.shape[0] / 1000}]
    # train forest
    rfs = GridSearchCV(RandomForestRegressor(50, max_depth=8, n_jobs=-1),
                      param_grid, cv=kf, n_jobs=-1)
    rfs.fit(x, y)
    rf = rfs.best_estimator_
    tree.export_graphviz(rf.estimators_[0], out_file='tree.dot')
#     rf = RandomForestRegressor(50, max_depth=13,
#                                min_samples_leaf=5 * x.shape[0] / 1000, n_jobs=-1)
#     rf.fit(x, y)
    print("best random forest parameters: " + str(rf.get_params()))
    # return the best forest
    return rf


class TrainForest(luigi.Task):
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "rf_" +
                                              self.file_name_prefix_training))

    def requires(self):
        return ParameterFile(self.file_name_prefix_training), \
            ReflectanceFile(self.file_name_prefix_training)

    def run(self):
        train_params, train_refl = extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        train_params = train_params[:, 0:4]
        rf = train(train_refl, train_params)
        # save the forest
        f = self.output().open('w')
        pickle.dump(rf, f)
        f.close()


class TrainForestForwardModel(luigi.Task):
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "rf_forward_" +
                                              self.file_name_prefix_training))

    def requires(self):
        return ParameterFile(self.file_name_prefix_training), \
            ReflectanceFile(self.file_name_prefix_training)


    def run(self):
        train_params, train_refl = extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        train_params = train_params[:, 0:4]
        rf = train(train_params, train_refl)
        # save the forest
        f = open(self.output().path, 'w')
        pickle.dump(rf, f)
        f.close()


class DataAugmentation(luigi.Task):
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "augmented_" +
                                              self.file_name_prefix_training +
                                              "_params" +
                                              ".npy")), \
               luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "augmented_" +
                                              self.file_name_prefix_training +
                                              "reflectances" +
                                              ".npy"))

    def requires(self):
        return TrainForestForwardModel(self.file_name_prefix_training), \
            ParameterFile(self.file_name_prefix_training)

    def run(self):
        # n samples shall be created
        n = 10 ** 5
        # load forward forest
        rf_file = self.input()[0].open('r')
        forward_rf = pickle.load(rf_file)
        # find ranges in which samples shall be drawn
        params = np.load(self.input()[1].path)
        params = params[:, 0:4]
        params_min = np.min(params, axis=0)
        params_max = np.max(params, axis=0)
        # create parameter array
        enh_params = np.zeros((n, params.shape[1]))

        for i in range(n):
            # draw uniform sample in parameter range
            enh_param = (params_max - params_min) * \
                    np.random.random_sample(params_min.shape) + params_min
            for e in enh_param:
                noise = np.random.normal(e, e * 0.1)
                e += noise
            enh_params[i, :] = enh_param

        enh_refls = forward_rf.predict(enh_params)

        np.save(self.output()[0].path, enh_params)
        np.save(self.output()[1].path, enh_refls)


class TrainForestAugmentedData(luigi.Task):
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "rf_augmented_" +
                                              self.file_name_prefix_training))

    def requires(self):
        return DataAugmentation(self.file_name_prefix_training)

    def run(self):
        train_params, train_refl = extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        train_params = train_params[:, 0:4]
        rf = train(train_refl, train_params)
        # save the forest
        f = open(self.output().path, 'w')
        pickle.dump(rf, f)
        f.close()



class CalculateWeightsFromSegmentation(luigi.Task):
    imageName = luigi.Parameter()
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "weights_" +
                                              self.file_name_prefix_training +
                                              "_image_" + self.imageName +
                                              ".npy"))

    def requires(self):
        return ParameterFile(self.file_name_prefix_training), \
            ReflectanceFile(self.file_name_prefix_training), \
            ppt.PreprocessMSI(imageName=self.imageName), \
            ppt.SegmentMSI(imageName=self.imageName)

    def run(self):
        train_params, train_refl = extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        # get weights
        reader = NrrdReader()
        msi = reader.read(self.input()[2].path)
        segmentation = reader.read(self.input()[3].path).get_image()
        msimani.apply_segmentation(msi, segmentation)
        non_masked_elemnts = imgmani.remove_masked_elements(msi.get_image())
        # collapsed_image = np.ma.compress_rows(collapsed_image)
        weights = calc_weights(train_refl, non_masked_elemnts)
        assert(weights.shape[0] == train_refl.shape[0])
        np.save(self.output().path, weights)


class TrainForestDA(luigi.Task):
    imageName = luigi.Parameter()
    file_name_prefix_training = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "rf_" +
                                              self.imageName + "_" +
                                              self.file_name_prefix_training))

    def requires(self):
        return ParameterFile(self.file_name_prefix_training), \
            ReflectanceFile(self.file_name_prefix_training), \
            CalculateWeightsFromSegmentation(imageName=self.imageName,
                    file_name_prefix_training=self.file_name_prefix_training)

    def run(self):
        train_params, train_refl = extract_mc_data(self.input()[0].path,
                                                   self.input()[1].path)
        train_params = train_params[:, 0:4]
        # get weights
        weights = np.load(self.input()[2].path)
        # unfortunately no grid search can be used, because it doesn't
        # allow the additional weights paramter
        rf = RandomForestRegressor(50, max_depth=13, max_features=8,
                                   min_samples_leaf=5, n_jobs=-1)
        rf.fit(train_refl, train_params, weights)
        # save the forest
        f = open(self.output().path, 'w')
        pickle.dump(rf, f)
        f.close()


