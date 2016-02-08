'''
Created on Aug 27, 2015

@author: wirkert
'''

import os

ROOT_FOLDER = "../.."
LOG_FOLDER = os.path.join(ROOT_FOLDER, "log")
DATA_FOLDER = os.path.join(ROOT_FOLDER, "data")
FLAT_FOLDER = os.path.join(DATA_FOLDER, "flatfields")
DARK_FOLDER = os.path.join(DATA_FOLDER, "dark")
SMALL_BOWEL_FOLDER = os.path.join(DATA_FOLDER,
                                  "small_bowel_images")
LIVER_FOLDER = os.path.join(DATA_FOLDER, "liver_images")
SPECTROMETER_FOLDER = os.path.join(DATA_FOLDER, "spectrometer_measurements")
RESULTS_FOLDER = os.path.join(ROOT_FOLDER, "results")
INTERMEDIATES_FOLDER = os.path.join(RESULTS_FOLDER, "intermediate")
MC_DATA_FOLDER = os.path.join(INTERMEDIATES_FOLDER, "mc_data")


def create_folder_if_necessary(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)

create_folder_if_necessary(INTERMEDIATES_FOLDER)
