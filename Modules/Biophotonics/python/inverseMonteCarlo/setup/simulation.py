# -*- coding: utf-8 -*-
"""
Created on Fri Feb  6 15:32:26 2015

@author: wirkert
"""



import numpy as np
from scipy.interpolate import interp1d
from scipy.stats import norm


nrSimulations = 10000

def normal():
    #%% intialization
    print('initialize...')

    # evaluated parameters for MC

    # blood volume fraction range
    samplesBVF = 20
    BVFs = np.linspace(0.01, 0.20, samplesBVF)
    # scattering particles volume fraction range
    samplesVs = 20
    Vss  = np.linspace(0.04, 0.60, samplesVs)
    # thickness of mucosa [m] range
    samplesD = 5
    ds   = np.linspace(250 * 10**-6, 735 * 10**-6, samplesD)
    #ds   = np.linspace(500 * 10**-6, 500 * 10**-6, samplesD)
    # haemoglobin saturation
    samplesSaO2 = 6
    SaO2s= np.linspace(0.3, 0.9, samplesSaO2)
    # radius of scattering particles [m]
    samplesR = 1
    rs   = np.linspace(0.4 * 10**-6, 0.4 * 10**-6, samplesR)
    # number of photons for one MC simulation run
    photons = 1 * 10**6
    # the wavelengths [m] for which the reflectance spectrum shall be evaluated
    #wavelengths = np.linspace(470,700,23) * 10**-9
    wavelengths = np.array([580, 470, 660, 560, 480, 611, 600, 700]) * 10**-9

    # The full width at half maximum [m] of the used imaging systems filters
    FWHM = 20 * 10**-9

    print('create reference data...')
    #reference data

    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt("data/haemoglobin.txt", skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:,0]   = haemoLUT[:,0] * 10**-9
    haemoLUT[:,1:2] = haemoLUT[:,1:2] * 10**2
    eHbO2 = interp1d(haemoLUT[:,0], haemoLUT[:,1])
    eHb   = interp1d(haemoLUT[:,0], haemoLUT[:,2])
    # to account for the FWHM of the used filters, compute convolution
    # see http://en.wikipedia.org/wiki/Full_width_at_half_maximum
    filterResponse = norm(loc = 0, scale = FWHM / 2.355)
    # parse at 20 locations
    x = np.linspace(filterResponse.ppf(0.01),
                   filterResponse.ppf(0.99), 20)
    filterResponse_table = filterResponse.pdf(x)
    # TODO verify if this normalization is correct!
    # filterResponse_table = filterResponse_table / sum(filterResponse_table)
    haemoLUT[:, 1] = np.convolve(haemoLUT[:, 1], filterResponse_table, 'same')
    haemoLUT[:, 2] = np.convolve(haemoLUT[:, 2], filterResponse_table, 'same')
    #plt.plot(haemoLUT[:, 0], haemoLUT[:, 1], '--', linewidth=2)
    #plt.show()


    nrSamples    = samplesBVF * samplesD * samplesR * samplesSaO2 * samplesVs


    return BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations



def noisy():
    #%% intialization
    print('initialize...')

    # evaluated parameters for MC

    # blood volume fraction range
    samplesBVF = 100
    BVFs = np.linspace(0.01, 0.30, samplesBVF)
    # scattering particles volume fraction range
    samplesVs = 100
    Vss  = np.linspace(0.04, 0.60, samplesVs)
    # thickness of mucosa [m] range
    samplesD = 5
    ds   = np.linspace(250 * 10**-6, 735 * 10**-6, samplesD)
    #ds   = np.linspace(500 * 10**-6, 500 * 10**-6, samplesD)
    # haemoglobin saturation
    samplesSaO2 = 6
    SaO2s= np.linspace(0.0, 1.0, samplesSaO2)
    # radius of scattering particles [m]
    samplesR = 1
    rs   = np.linspace(0.4 * 10**-6, 0.4 * 10**-6, samplesR)
    # number of photons for one MC simulation run
    photons = 1 * 10**6
    # the wavelengths [m] for which the reflectance spectrum shall be evaluated
    #wavelengths = np.linspace(470,700,23) * 10**-9
    wavelengths = np.array([580, 470, 660, 560, 480, 611, 600, 700]) * 10**-9

    # The full width at half maximum [m] of the used imaging systems filters
    FWHM = 20 * 10**-9

    print('create reference data...')
    #reference data

    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt("data/haemoglobin.txt", skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:,0]   = haemoLUT[:,0] * 10**-9
    haemoLUT[:,1:2] = haemoLUT[:,1:2] * 10**2
    eHbO2 = interp1d(haemoLUT[:,0], haemoLUT[:,1])
    eHb   = interp1d(haemoLUT[:,0], haemoLUT[:,2])
    # to account for the FWHM of the used filters, compute convolution
    # see http://en.wikipedia.org/wiki/Full_width_at_half_maximum
    filterResponse = norm(loc = 0, scale = FWHM / 2.355)
    # parse at 20 locations
    x = np.linspace(filterResponse.ppf(0.01),
                   filterResponse.ppf(0.99), 20)
    filterResponse_table = filterResponse.pdf(x)
    # TODO verify if this normalization is correct!
    # filterResponse_table = filterResponse_table / sum(filterResponse_table)
    haemoLUT[:, 1] = np.convolve(haemoLUT[:, 1], filterResponse_table, 'same')
    haemoLUT[:, 2] = np.convolve(haemoLUT[:, 2], filterResponse_table, 'same')
    #plt.plot(haemoLUT[:, 0], haemoLUT[:, 1], '--', linewidth=2)
    #plt.show()


    nrSamples    = samplesBVF * samplesD * samplesR * samplesSaO2 * samplesVs


    return BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations



def perfect():
    #%% intialization
    print('initialize...')

    # evaluated parameters for MC

    # blood volume fraction range
    samplesBVF = 100
    BVFs = np.linspace(0.01, 0.30, samplesBVF)
    # scattering particles volume fraction range
    samplesVs = 100
    Vss  = np.linspace(0.04, 0.60, samplesVs)
    # thickness of mucosa [m] range
    samplesD = 1
    #ds   = np.linspace(250 * 10**-6, 735 * 10**-6, samplesD)
    ds   = np.linspace(500 * 10**-6, 500 * 10**-6, samplesD)
    # haemoglobin saturation
    samplesSaO2 = 1
    SaO2s= np.linspace(0.7, 0.7, samplesSaO2)
    # radius of scattering particles [m]
    samplesR = 1
    rs   = np.linspace(0.4 * 10**-6, 0.4 * 10**-6, samplesR)
    # number of photons for one MC simulation run
    photons = 1 * 10**6
    # the wavelengths [m] for which the reflectance spectrum shall be evaluated
    #wavelengths = np.linspace(470,700,23) * 10**-9
    wavelengths = np.array([580, 470, 660, 560, 480, 611, 600, 700]) * 10**-9

    # The full width at half maximum [m] of the used imaging systems filters
    FWHM = 20 * 10**-9

    print('create reference data...')
    #reference data

    # table with wavelength at 1st row,
    # HbO2 molar extinction coefficient [cm**-1/(moles/l)] at 2nd row,
    # Hb molar extinction coefficient [cm**-1/(moles/l)] at 3rd row
    haemoLUT = np.loadtxt("data/haemoglobin.txt", skiprows=2)
    # we calculate everything in [m] instead of [nm] and [1/cm]
    haemoLUT[:,0]   = haemoLUT[:,0] * 10**-9
    haemoLUT[:,1:2] = haemoLUT[:,1:2] * 10**2
    eHbO2 = interp1d(haemoLUT[:,0], haemoLUT[:,1])
    eHb   = interp1d(haemoLUT[:,0], haemoLUT[:,2])
    # to account for the FWHM of the used filters, compute convolution
    # see http://en.wikipedia.org/wiki/Full_width_at_half_maximum
    filterResponse = norm(loc = 0, scale = FWHM / 2.355)
    # parse at 20 locations
    x = np.linspace(filterResponse.ppf(0.01),
                   filterResponse.ppf(0.99), 20)
    filterResponse_table = filterResponse.pdf(x)
    # TODO verify if this normalization is correct!
    # filterResponse_table = filterResponse_table / sum(filterResponse_table)
    haemoLUT[:, 1] = np.convolve(haemoLUT[:, 1], filterResponse_table, 'same')
    haemoLUT[:, 2] = np.convolve(haemoLUT[:, 2], filterResponse_table, 'same')
    #plt.plot(haemoLUT[:, 0], haemoLUT[:, 1], '--', linewidth=2)
    #plt.show()

    nrSamples    = samplesBVF * samplesD * samplesR * samplesSaO2 * samplesVs

    return BVFs, Vss, ds, SaO2s, rs, nrSamples, photons, wavelengths, FWHM, eHbO2, eHb, nrSimulations
