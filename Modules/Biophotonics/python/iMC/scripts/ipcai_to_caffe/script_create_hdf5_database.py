import h5py, os
import pandas as pd

from regression.preprocessing import preprocess


def create_hdf5(path_to_simulation_results, hdf5_name):

    df = pd.read_csv(path_to_simulation_results, header=[0, 1])

    X, y = preprocess(df, snr=10.)
    y = y.values

    with h5py.File(hdf5_name,'w') as H:
        H.create_dataset('data', data=X )  # note the name X given to the dataset!
        H.create_dataset('label', data=y )  # note the name y given to the dataset!
    with open(hdf5_name + '_list.txt','w') as L:
        L.write(hdf5_name)  # list all h5 files you are going to use


data_root = "/media/wirkert/data/Data/2016_02_02_IPCAI/results/intermediate"

TRAIN_IMAGES = os.path.join(data_root,
                            "ipcai_revision_colon_mean_scattering_train_all_spectrocam.txt")
TEST_IMAGES = os.path.join(data_root,
                           "ipcai_revision_colon_mean_scattering_test_all_spectrocam.txt")

create_hdf5(TRAIN_IMAGES, "ipcai_train_hdf5.h5")
create_hdf5(TEST_IMAGES, "ipcai_test_hdf5.h5")
