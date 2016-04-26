'''
Created on Sep 8, 2015

@author: wirkert
'''
import unittest
import filecmp
import os

from mc.sim import MciWrapper, SimWrapper, \
                   get_total_reflectance, get_diffuse_reflectance


path_to_gpumcml = "/home/wirkert/workspace/monteCarlo/gpumcml/" + \
                                        "fast-gpumcml/gpumcml.sm_20"
skip_gpu_tests = not os.path.exists(path_to_gpumcml)


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

    def tearDown(self):
        os.remove(self.mci_filename)
        mcml_path, mcml_file = os.path.split(path_to_gpumcml)
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

    @unittest.skipIf(skip_gpu_tests, "skip if gpumcml not installed")
    def test_sim_wrapper(self):
        mcml_path, mcml_file = os.path.split(path_to_gpumcml)
        if os.path.isfile(path_to_gpumcml):
            self.mci_wrapper.create_mci_file()
            sim_wrapper = SimWrapper()
            sim_wrapper.set_mci_filename(self.mci_filename)
            sim_wrapper.set_mcml_executable(path_to_gpumcml)
            sim_wrapper.run_simulation()
            self.assertTrue(os.path.isfile(os.path.join(mcml_path,
                                                        self.mco_filename)),
                        "mco file was created")

    @unittest.skipIf(skip_gpu_tests, "skip if gpumcml not installed")
    def test_mci_wrapper_book_example(self):
        """see if our result matches the one from
        Biomedical Optics
        Principles and Imaging
        page 55 (Table 3.1)"""
        # create a book_p55_mci which shall create a mci file
        book_p55_mci = MciWrapper()
        book_p55_mci.set_mci_filename(self.mci_filename)
        book_p55_mci.set_mco_filename(self.mco_filename)
        book_p55_mci.set_nr_photons(10**6)
        book_p55_mci.add_layer(1, 1000, 9000, 0.75, 0.0002)

        mcml_path, mcml_file = os.path.split(path_to_gpumcml)
        if os.path.isfile(path_to_gpumcml):
            book_p55_mci.create_mci_file()
            sim_wrapper = SimWrapper()
            sim_wrapper.set_mci_filename(self.mci_filename)
            sim_wrapper.set_mcml_executable(path_to_gpumcml)
            sim_wrapper.run_simulation()
            self.assertTrue(os.path.isfile(mcml_path + "/" + self.mco_filename),
                        "mco file was created")
            refl = get_diffuse_reflectance(os.path.join(mcml_path,
                                                        self.mco_filename))
            self.assertAlmostEqual(refl, 0.09734, 3,
                                   "correct reflectance determined " +
                                   "according to book table 3.1")

    @unittest.skipIf(skip_gpu_tests, "skip if gpumcml not installed")
    def test_mci_wrapper_book_example_2(self):
        """see if our result matches the one from
        Biomedical Optics
        Principles and Imaging
        page 56 (Table 3.2)"""
        # create a book_p56_mci which shall create a mci file
        book_p56_mci = MciWrapper()
        book_p56_mci.set_mci_filename(self.mci_filename)
        book_p56_mci.set_mco_filename(self.mco_filename)
        book_p56_mci.set_nr_photons(10**6)
        book_p56_mci.add_layer(1.5, 1000, 9000, 0., 1)

        mcml_path, mcml_file = os.path.split(path_to_gpumcml)
        if os.path.isfile(path_to_gpumcml):
            book_p56_mci.create_mci_file()
            sim_wrapper = SimWrapper()
            sim_wrapper.set_mci_filename(self.mci_filename)
            sim_wrapper.set_mcml_executable(path_to_gpumcml)
            sim_wrapper.run_simulation()
            self.assertTrue(os.path.isfile(mcml_path + "/" + self.mco_filename),
                        "mco file was created")
            refl = get_total_reflectance(os.path.join(mcml_path,
                                                      self.mco_filename))
            self.assertAlmostEqual(refl, 0.26, delta=0.01,
                                   msg="correct reflectance determined " +
                                   "according to book table 3.2")
