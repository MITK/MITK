'''
Created on Sep 8, 2015

@author: wirkert
'''

import unittest

from mc.usuag import Ua, UsG


class test_ua(unittest.TestCase):

    def setUp(self):
        self.ua = Ua()
        self.ua532 = self.ua.eval(532.*10 ** -9) / 100.
        self.ua800 = self.ua.eval(800.*10 ** -9) / 100.

    def test_uA532(self):
        self.assertTrue(3. < self.ua532 < 4., "test if calculated ua takes " +
                        "reasonable (according to \"Determination of optical" +
                        " properties of normal and adenomatous human colon " +
                        "tissues in vitro using integrating sphere " +
                        "techniques\") values")

    def test_uA800(self):
        self.assertTrue(0.05 < self.ua800 < 0.15, "test if calculated ua " +
                        "takes reasonable (according to \"Differences in" +
                        " optical properties between healthy and " +
                        "pathological human colon tissues using a Ti:sapphire" +
                        " laser: an in vitro study using the Monte Carlo " +
                        "inversion technique\") values")

    def test_saO2_makes_difference(self):
        self.ua.saO2 = 1.0
        self.assertNotAlmostEqual(self.ua532,
                                  self.ua.eval(532.*10 ** -9) / 100.,
                                  msg="changing oxygenation changes result")
