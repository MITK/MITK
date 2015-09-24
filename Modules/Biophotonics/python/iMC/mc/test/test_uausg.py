'''
Created on Sep 8, 2015

@author: wirkert
'''

import unittest

from mc.usuag import Ua, UsgMie


class test_ua(unittest.TestCase):

    def setUp(self):
        self.ua_sm = Ua()
        self.ua532 = self.ua_sm(532.*10 ** -9) / 100.
        self.ua800 = self.ua_sm(800.*10 ** -9) / 100.

    def test_uA532(self):
        self.assertTrue(3. < self.ua532 < 4., "test if calculated ua_sm takes " +
                        "reasonable values " +
                        "(according to \"Determination of optical" +
                        " properties of normal and adenomatous human colon " +
                        "tissues in vitro using integrating sphere " +
                        "techniques\")")

    def test_uA800(self):
        self.assertTrue(0.05 < self.ua800 < 0.15, "test if calculated ua_sm " +
                        "takes reasonable values " +
                        "(according to \"Differences in" +
                        " optical properties between healthy and " +
                        "pathological human colon tissues using a Ti:sapphire" +
                        " laser: an in vitro study using the Monte Carlo " +
                        "inversion technique\")")

    def test_saO2_makes_difference(self):
        self.ua_sm.saO2 = 1.0
        self.assertNotAlmostEqual(self.ua532,
                                  self.ua_sm(532.*10 ** -9) / 100.,
                                  msg="changing oxygenation changes result")


class test_us(unittest.TestCase):

    def setUp(self):
        self.usg = UsgMie()
        self.usg.dsp = 0.015
        self.usg470 = self.usg(470.*10 ** -9)
        self.usg700 = self.usg(700.*10 ** -9)

    def test_us470nm(self):
        reduced_us470nm = self.usg470[0] * (1 - self.usg470[1]) / 100.
        self.assertTrue(5. < reduced_us470nm < 10.,
                        "test if calculated us' can take reasonable values"
                        + "(according to \"Ex Vivo Optical Properties of"
                        + " Human Colon Tissue\"). Calculated value is " +
                        str(reduced_us470nm) + "1 / cm")


    def test_us700nm(self):
        reduced_us700nm = self.usg700[0] * (1 - self.usg700[1]) / 100.
        self.assertTrue(5. < reduced_us700nm < 10.,
                        "test if calculated us' can take reasonable values" +
                        "(according to \"Ex Vivo Optical Properties of " +
                        "Human Colon Tissue\")." +
                        " Calculated value is " + str(reduced_us700nm) +
                        "1 / cm")

    def test_from_book(self):
        """see if our result matches the one from
        Biomedical Optics
        Principles and Imaging
        page 23"""
        self.usg.r = 579. * 10 ** -9 / 2.
        self.usg.n_particle = 1.57
        self.usg.n_medium = 1.33
        self.usg.dsp = 0.002 * 1.0 * 10 ** 3 / (1.05 * 10 ** 3)
        us, g = self.usg(400. * 10 ** -9)


        self.assertAlmostEqual(us * 10 ** -2, 100., places=1,
                               msg="scattering coefficient matches book " + \
                               "example")
        self.assertAlmostEqual(g, 0.916, places=3,
                               msg="anisotropy factor matches book example")

