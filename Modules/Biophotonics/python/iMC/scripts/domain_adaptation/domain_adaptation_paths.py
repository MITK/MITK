

import os

ROOT_FOLDER = "/media/wirkert/data/Data/2016_03_Domain_Adaptation"
LOG_FOLDER = os.path.join(ROOT_FOLDER, "log")
DATA_FOLDER = os.path.join(ROOT_FOLDER, "data")
RESULTS_FOLDER = os.path.join(ROOT_FOLDER, "results")
INTERMEDIATES_FOLDER = os.path.join(RESULTS_FOLDER, "intermediate")
MC_DATA_FOLDER = os.path.join(INTERMEDIATES_FOLDER, "mc_data")


def create_folder_if_necessary(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)

create_folder_if_necessary(INTERMEDIATES_FOLDER)
create_folder_if_necessary(LOG_FOLDER)
