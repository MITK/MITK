# -*- coding: utf-8 -*-


import math
from pymiecoated import Mie

#%% setup and create derived paramters

def mieMonteCarlo(wavelength = 450*10**-9, r = 300*10**-9, Vs = 0.1, nParticle = 1.46, nMedium = 1.36):
    """
    Calculate the scattering parameters relevant for monte carlo simulation.

    Needs pymiecoated: https://code.google.com/p/pymiecoated/
    These are calculate scattering coefficient [1/cm] and anisotropy factor for given:

    Args
    ____
    wavelength:
        wavelength of the incident light [m]
    r:
        radius of the particle [m]
    Vs:
        volume fraction of scattering particles
    nParticle:
        refractive index of the particle that the light wave is scattered on
        (default value is the refractive index of collagen)
    nMedium:
        refractive index of the surronding medium (default is that of colonic
        mucosal tissue)

    Returns:
    ____
    {'us', 'g'}:
        scattering coefficient us [1/m] and anisotropy factor g

    TODO:
    _____
        Additional input parameter specifying a FWHM for the wavelength to simulate the
        scattering for a broad filter
    """

    # create derived parameters
    sizeParamter = 2 * math.pi * r / wavelength
    nRelative    = nParticle / nMedium

    #%% execute mie and create derived parameters

    mie = Mie(x=sizeParamter, m=complex(nRelative,0.0))  # 0.0 complex for no attenuation

    A  = math.pi * r**2 # geometrical cross sectional area
    cs = mie.qsca() * A # scattering cross section

    us = Vs / (4/3 * r**3 * math.pi) * cs # scattering coefficient [m⁻1]

    return {'us': us, 'g': mie.asy()}

def mieMonteCarlo_FWHM(wavelength = 450*10**-9, r = 300*10**-9, Vs = 0.1, nParticle = 1.46, nMedium = 1.36, FWHM = 0):
    vl = mieMonteCarlo(wavelength - FWHM / 2, r, Vs, nParticle, nMedium)
    vr = mieMonteCarlo(wavelength + FWHM / 2, r, Vs, nParticle, nMedium)
    vc = mieMonteCarlo(wavelength,            r, Vs, nParticle, nMedium)

    us = (0.5 * vl['us'] + vc['us'] + 0.5 * vr['us']) / 2
    g  = (0.5 * vl['g']  + vc['g']  + 0.5 * vr['g']) / 2
    return {'us': us, 'g': g}

if __name__ == "__main__":

    usg = mieMonteCarlo(wavelength = 450*10**-9, r = 300*10**-9, Vs = 0.1, nParticle = 1.46, nMedium = 1.36)

    #%% print results
    print("Scattering coefficient in [1/cm]: " + str(usg['us'] * 10**-2) + "; expected: 589.36")
    print("Anisotropy factor: " + str(usg['g']) + "; expected: 0.88")
