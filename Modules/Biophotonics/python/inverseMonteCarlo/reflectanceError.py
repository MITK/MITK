# -*- coding: utf-8 -*-
"""
Created on Tue Feb 10 15:50:13 2015

@author: wirkert
"""

from scipy.interpolate import RectBivariateSpline
import numpy as np

class ReflectanceError:
    """
    calculate the fit error of a given reflectance and examples of reflectances.
    """

    def __init__(self, BVFs, Vss, reflectances):
        """
        intialize the reflectance error calculation.

        Arguments
        _________
        BVFs:
            array storing all the differnet values for BVF (Blood Volume Fraction)
        Vss:
            array storing all the different values Vs (Volume fraction of scattering particles)
        reflectances:
            grid of dimension BVFs.shape[0] x Vss.shape[0] x wavelenghts with reflectance values
        """
        nr_wavelengths = reflectances.shape[2]

        # create array of bivariate splines, one for each wavelength
        self.RBSs = np.empty(nr_wavelengths, dtype=object)

        for i in np.arange(nr_wavelengths):
            self.RBSs[i] = RectBivariateSpline(BVFs,
                  Vss,
                  reflectances[:,:,i])

    def setReflectanceToMatch(self, reflectance):
        """
        set the baseline reflectance. the l2 distance to this reflectance will be output
        """
        self.reflectance = reflectance

    def reflectanceForX(self, xy):
        result = np.zeros(len(self.reflectance))
        for idx, rbsi in enumerate(self.RBSs):
            result[idx] = rbsi(xy[0], xy[1])
        return result

    def f(self, xy):
        """
        return l2 norm of reflectance implied by x and self.reflectances
        """
        quadraticError = 0
        # build quadratic error of differences between reflectances and
        for refli, rbsi in zip(self.reflectance, self.RBSs):
            quadraticError += (refli - rbsi(xy[0], xy[1], grid = False))**2

        return quadraticError

    def df(self, xy):
        """
        return derivate of l2 norm of reflectance implied by x and self.reflectances
        """
        quadraticErrorDerivate = [0,0]
        for refli, rbsi in zip(self.reflectance, self.RBSs):
            u = 2*(rbsi(xy[0], xy[1], grid = False) - refli)
            # dx
            quadraticErrorDerivate[0] += \
                u*rbsi(xy[0], xy[1], dx=1, grid = False)
            #dy
            quadraticErrorDerivate[1] += \
                u*rbsi(xy[0], xy[1], dy=1, grid = False)
        return quadraticErrorDerivate
