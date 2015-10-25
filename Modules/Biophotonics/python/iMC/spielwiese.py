'''
Created on Oct 19, 2015

@author: wirkert
'''

import pickle
import numpy as np


def save_as_np(filename, prefix):
    f = open(filename)
    batch = pickle.load(f)
    np.save("/media/wirkert/data/Data/temp/" + prefix + "_reflectances",
            batch.reflectances)
    np.save("/media/wirkert/data/Data/temp/" + prefix + "_saO2",
            batch.layers[0][:, 1])

if __name__ == '__main__':

    save_as_np("/media/wirkert/data/Data/" +
        "2015_11_12_IPCAI_in_silico/processed/" +
        "generic_tissue_train_all_camera.imc",
        "hard_train_2")

    save_as_np("/media/wirkert/data/Data/" +
        "2015_11_12_IPCAI_in_silico/processed/" +
        "generic_tissue_gaussian_all_camera.imc",
        "gaussian_train_2")
