"""
This file contains a singleton class which manages the paths set for evaluating
the scripts.

Also it contains some utility methods.
"""

import os

import numpy as np


class ScriptCommons(object):
    """
    The commonly shared paths to the data/log/results... folders.
    + additional commonly available data as recorded wavelengths
    this is a singleton pattern copied from
    http://python-3-patterns-idioms-test.readthedocs.org/en/latest/Singleton.html
    """

    class __ScriptCommons_Singleton:

        def __init__(self):

            self.root = os.path.join("..", "..")

            self.dirs = {"LOG_FOLDER": "log",
                         "DATA_FOLDER": "data",
                         "RESULTS_FOLDER": "results"}

            self.dirs["FLAT_FOLDER"] = os.path.join(self.dirs["DATA_FOLDER"],
                                                    "flatfields")
            self.dirs["DARK_FOLDER"] = os.path.join(self.dirs["DATA_FOLDER"],
                                                    "dark")

            self.dirs["INTERMEDIATES_FOLDER"] = os.path.join(
                    self.dirs["RESULTS_FOLDER"], "intermediate")

            self.dirs["MC_DATA_FOLDER"] = os.path.join(
                    self.dirs["INTERMEDIATES_FOLDER"], "mc_data")

            self.other = {"RECORDED_WAVELENGTHS": np.array([580, 470,
                                                            660, 560,
                                                            480, 511,
                                                            600, 700])
                                                  * 10 ** -9}

        def create_folders(self):
            """
            Create all folders listed in self.folders if not existing
            """
            for f in self.dirs:
                create_folder_if_necessary(self.get_full_dir(f))

        def set_root(self, root):
            self.root = root

        def get_root(self):
            return self.root

        def add_dir(self, key, new_dir):
            """
            Add/replace a directory to the singletons list.
            Directories can be returned with get_dir and with their full path by
            calling get_full_dir

            :param key: the key under which it shall be retrievable
            :param new_dir: the directory to add to the list
            """
            self.dirs[key] = new_dir

        def get_dir(self, key):
            return self.dirs[key]

        def get_full_dir(self, key):
            return os.path.join(self.get_root(), self.get_dir(key))

    instance = None

    def __new__(cls): # __new__ always a classmethod
        if not ScriptCommons.instance:
            ScriptCommons.instance = ScriptCommons.__ScriptCommons_Singleton()
        return ScriptCommons.instance

    def __getattr__(self, name):
        return getattr(self.instance, name)

    def __setattr__(self, name):
        return setattr(self.instance, name)


def create_folder_if_necessary(folder):
    """
    :param folder: create the folder folder if necessary (not already existing)
    """
    if not os.path.exists(folder):
        os.makedirs(folder)
