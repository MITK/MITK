'''
Created on Sep 8, 2015

@author: wirkert
'''
import unittest
import filecmp
import os

from mc.sim import MciWrapper, SimWrapper

class Test(unittest.TestCase):


    def setUp(self):
        self.mci_filename = "temp.mci"
        self.mco_filename = "temp.mco"
        # create a mci_wrapper which shall create a mci file
        self.mci_wrapper = MciWrapper()
        self.mci_wrapper.set_mci_filename(self.mci_filename)
        self.mci_wrapper.set_mco_filename(self.mco_filename)
        self.mci_wrapper.set_nr_photons(10 ** 6)
        self.mci_wrapper.add_layer(1.0, 2.1, 3.2, 4.3, 5.4)
        self.mci_wrapper.add_layer(6.5, 7.8, 8.9, 9.10, 10.11)
        self.mci_wrapper.add_layer(100.1001, 101.10001, 102.100001,
                               103.1000001, 104.10000001)
        self.mci_wrapper.set_layer(1, 1, 1, 1, 1, 1)
        # expected mci file
        self.correct_mci_filename = "./mc/data/correct.mci"
        # path to the externaly installed mcml simulation. This is machine
        # dependent. Thus tests depending on the execution of mcml will only
        # be performed if this file exists.
        # Should the file be located somewhere else on your computer,
        # change this path to your actual location.
        self.path_to_gpumcml = "/home/wirkert/workspace/monteCarlo/gpumcml/" + \
                                        "fast-gpumcml/gpumcml.sm_20"

    def tearDown(self):
        os.remove(self.mci_filename)
        mcml_path, mcml_file = os.path.split(self.path_to_gpumcml)
        created_mco_file = mcml_path + "/" + self.mco_filename
        if os.path.isfile(created_mco_file):
            os.remove(created_mco_file)

    def test_mci_wrapper(self):
        self.mci_wrapper.create_mci_file()
        self.assertTrue(os.path.isfile(self.mci_filename),
                        "mci file was created")
        self.assertTrue(filecmp.cmp(self.mci_filename,
                                    self.correct_mci_filename, shallow=False),
                        "the written mci file is the same as the stored " +
                        "reference file")

    def test_sim_wrapper(self):
        mcml_path, mcml_file = os.path.split(self.path_to_gpumcml)
        if os.path.isfile(self.path_to_gpumcml):
            self.mci_wrapper.create_mci_file()
            sim_wrapper = SimWrapper()
            sim_wrapper.set_mci_filename(self.mci_filename)
            sim_wrapper.set_mcml_executable(self.path_to_gpumcml)
            sim_wrapper.run_simulation()
            self.assertTrue(os.path.isfile(mcml_path + "/" + self.mco_filename),
                        "mco file was created")
