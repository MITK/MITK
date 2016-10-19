'''
Created on Sep 8, 2015

This file contains methods which wrap the mcml simulation so it can be
conveniently called from python. One example for a mcml simulation would be
gpumcml:
https://code.google.com/p/gpumcml/

@author: wirkert
'''

import os
import contextlib
import logging

import subprocess32

""" helper method to change to the correct path and back again """
@contextlib.contextmanager
def cd(newPath):
    savedPath = os.getcwd()
    os.chdir(newPath)
    yield
    os.chdir(savedPath)


class MciWrapper(object):
    '''
    this class provides a wrapper to the mcml monte carlo file.
    Its purpose is to create a .mci file which the mcml simulation can use to
    create the simulation
    '''

    def set_mci_filename(self, mci_filename):
        self.mci_filename = mci_filename

    def set_mco_filename(self, mco_filename):
        """path of the mco file.
        This can be either a path relative to the mcml executable
        or an absolute path.
        BUG: it seems that it can only be relative file name
        """
        self.mco_filename = mco_filename

    def set_nr_photons(self, nr_photons):
        self.nr_photons = nr_photons

    def add_layer(self, n=None, ua=None, us=None, g=None, d=None):
        """adds a layer below the currently existing ones.

        Arguments:
        n: Refraction index of medium
        ua: absorption coefficient [1/m]
        us: scattering coefficient [1/m]
        g: anisotropy factor
        d: thickness of layer [m]
        """
        if n is None:
            n = 1.
        if ua is None:
            ua = 0.
        if us is None:
            us = 0.
        if g is None:
            g = 1.
        if d is None:
            d = 500.*10 ** -6
        self.layers.append([n, ua, us, g, d])

    def set_layer(self, layer_nr, n, ua, us, g, d):
        """set a layer with a specific layer_nr (stariting with layer_nr 0).
        Note that the layer must already exist, otherwise an error will occure
        """
        self.layers[layer_nr] = [n, ua, us, g, d]

    def set_file_version(self, file_version):
        self.file_version = file_version

    def set_nr_runs(self, nr_runs):
        self.nr_runs = nr_runs

    def set_dz_dr(self, dz, dr):
        self.dz = dz
        self.dr = dr

    def set_nr_dz_dr_da(self, nr_dz, nr_dr, nr_da):
        self.nr_dz = nr_dz
        self.nr_dr = nr_dr
        self.nr_da = nr_da

    def set_n_medium_above(self, n_above):
        self.n_above = n_above

    def set_n_medium_below(self, n_below):
        self.n_below = n_below

    def create_mci_file(self):
        """this method creates the mci file at the location self.mci_filename"""
        open(self.mci_filename, 'a').close()
        f = open(self.mci_filename, 'w')
        # write general information
        f.write(str(self.file_version) + " # file version\n")
        f.write(str(self.nr_runs) + " # number of runs\n\n")
        # write the data for run
        f.write(self.mco_filename + " A # output filename, ASCII/Binary\n")
        f.write(str(self.nr_photons) + " # No. of photons\n")
        f.write(repr(self.dz) + " " + repr(self.dr) + " # dz, dr\n")
        f.write(repr(self.nr_dz) + " " +
                repr(self.nr_dr) + " " +
                repr(self.nr_da) + " # No. of dz, dr & da.\n\n")
        # write layer information
        f.write(str(len(self.layers)) + " # No. of layers\n")
        f.write("# n mua mus g d # One line for each layer\n")
        f.write(repr(self.n_above) + " # n for medium above.\n")
        for layer in self.layers:

            # factors (/100.; *100.) to convert to mcml expected units:
            f.write("%.3f" % layer[0] + " " +  # n
                    "%.5f" % (layer[1] / 100.) + " " +  # ua
                    "%.5f" % (layer[2] / 100.) + " " +  # us
                    "%.3f" % layer[3] + " " +  # g
                    "%.3f" % (layer[4] * 100.) + "\n")  # d
        f.write(repr(self.n_below) + " # n for medium below.\n")
        f.close()
        if not os.path.isfile(self.mci_filename):
            raise IOError("input file for monte carlo simulation not " +
                          "succesfully created")

    def __init__(self):
        # set standard parameters
        self.file_version = 1.0
        self.nr_photons = 10**6
        self.nr_runs = 1
        self.dz = 0.002
        self.dr = 2
        self.nr_dz = 500
        self.nr_dr = 1
        self.nr_da = 1
        self.n_above = 1.0
        self.n_below = 1.0
        # initialize to 0 layers
        self.layers = []


class SimWrapper(object):

    def set_mci_filename(self, mci_filename):
        """the full path to the input file. E.g. ./data/my.mci
        """
        self.mci_filename = mci_filename

    def set_mcml_executable(self, mcml_executable):
        """ the full path of the excutable. E.g. ./mcml/mcml.exe"""
        self.mcml_executable = mcml_executable

    def run_simulation(self):
        """this method runs a monte carlo simulation"""
        mcml_path, mcml_file = os.path.split(self.mcml_executable)
        abs_mci_filename = os.path.abspath(self.mci_filename)
        # note: the -A option makes gpumcml much faster, but is not available
        # in original mcml. Maybe a switch should be introduced here
        args = ("./" + mcml_file, "-A", abs_mci_filename)
        # switch to folder where mcml resides in and execute it.
        with cd(mcml_path):
            try:
                popen = subprocess32.Popen(args, stdout=subprocess32.PIPE)
                popen.wait(timeout=100)
            except:
                logging.error("couldn't run simulation")
                # popen.kill()

    def __init__(self):
        pass


def get_diffuse_reflectance(mco_filename):
    """
    extract reflectance from mco file.
    Attention: mco_filename specifies full path.

    Returns: the reflectance
    """
    with open(mco_filename) as myFile:
        for line in myFile:
            if "Diffuse reflectance" in line:
                return float(line.split(' ', 1)[0])


def get_specular_reflectance(mco_filename):
    """
    extract reflectance from mco file.
    Attention: mco_filename specifies full path.

    Returns: the reflectance
    """
    with open(mco_filename) as myFile:
        for line in myFile:
            if "Specular reflectance" in line:
                return float(line.split(' ', 1)[0])


def get_total_reflectance(mco_filename):
    """
    extract reflectance from mco file.
    Attention: mco_filename specifies full path.

    Returns: the reflectance
    """
    return get_diffuse_reflectance(mco_filename) + \
            get_specular_reflectance(mco_filename)

