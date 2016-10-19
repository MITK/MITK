
import os

import pandas as pd
import lmdb
import caffe

from regression.preprocessing import preprocess


def create_lmdb(path_to_simulation_results, lmdb_name):

    df = pd.read_csv(path_to_simulation_results, header=[0, 1])

    X, y = preprocess(df, snr=10.)
    y = y.values * 1000

    # We need to prepare the database for the size. We'll set it 10 times
    # greater than what we theoretically need. There is little drawback to
    # setting this too big. If you still run into problem after raising
    # this, you might want to try saving fewer entries in a single
    # transaction.
    map_size = X.nbytes * 10

    env = lmdb.open(lmdb_name, map_size=map_size)

    with env.begin(write=True) as txn:
        # txn is a Transaction object
        for i in range(X.shape[0]):
            datum = caffe.proto.caffe_pb2.Datum()
            datum.channels = X.shape[1]
            datum.height = 1
            datum.width = 1
            datum.data = X[i].tobytes()  # or .tostring() if numpy < 1.9
            datum.label = int(y[i])
            str_id = '{:08}'.format(i)

            # The encode is only essential in Python 3
            txn.put(str_id.encode('ascii'), datum.SerializeToString())


data_root = "/media/wirkert/data/Data/2016_02_02_IPCAI/results/intermediate"

TRAIN_IMAGES = os.path.join(data_root,
                            "ipcai_revision_colon_mean_scattering_train_all_spectrocam.txt")
TEST_IMAGES = os.path.join(data_root,
                           "ipcai_revision_colon_mean_scattering_test_all_spectrocam.txt")

create_lmdb(TRAIN_IMAGES, "ipcai_train_lmdb")
create_lmdb(TEST_IMAGES, "ipcai_test_lmdb")


