# -*- coding: utf-8 -*-
"""
Created on Fri Mar 27 19:12:40 2015

@author: wirkert
"""

import unittest
import helper.mieMonteCarlo as miemc


class test_mie(unittest.TestCase):

    def setUp(self):
        self.usg470 = miemc.mieMonteCarlo(wavelength = 450*10**-9, Vs = 0.04)
        self.usg700 = miemc.mieMonteCarlo(wavelength = 700*10**-9, Vs = 0.04)

    def test_us470nm(self):
        self.assertTrue(self.usg470['us'] * 10**-2 < 400)
        self.assertTrue(self.usg470['us'] * 10**-2 > 300)

    def test_us700nm(self):
        self.assertTrue(self.usg470['us'] * 10**-2 < 300)
        self.assertTrue(self.usg470['us'] * 10**-2 > 200)

    def test_g700nm(self):
        self.assertTrue(self.usg470['g'] > 0.9)




