'''
Created on Sep 8, 2015

@author: wirkert
'''

import unittest

from mc.usuag import Ua

class test_ua(unittest.TestCase):

    def setUp(self):
        self.ua_l2 = Ua()
        self.ua532 = self.ua_l2(532.*10 ** -9) / 100.
        self.ua800 = self.ua_l2(800.*10 ** -9) / 100.

    def test_uA532(self):
        self.assertTrue(3. < self.ua532 < 4., "test if calculated ua_l2 takes " +
                        "reasonable values " +
                        "(according to \"Determination of optical" +
                        " properties of normal and adenomatous human colon " +
                        "tissues in vitro using integrating sphere " +
                        "techniques\")")

    def test_uA800(self):
        self.assertTrue(0.05 < self.ua800 < 0.15, "test if calculated ua_l2 " +
                        "takes reasonable values " +
                        "(according to \"Differences in" +
                        " optical properties between healthy and " +
                        "pathological human colon tissues using a Ti:sapphire" +
                        " laser: an in vitro study using the Monte Carlo " +
                        "inversion technique\")")

    def test_saO2_makes_difference(self):
        self.ua_l2.saO2 = 1.0
        self.assertNotAlmostEqual(self.ua532,
                                  self.ua_l2(532.*10 ** -9) / 100.,
                                  msg="changing oxygenation changes result")

