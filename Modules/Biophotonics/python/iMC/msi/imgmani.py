'''
Created on Aug 28, 2015

@author: wirkert
'''

import numpy as np


def collapse_image(img):
    """ helper method which transorms the n x m x nrWavelengths image to a
    (n*m) x nrWavelength image.

    note that this function doesn't take an object of class Msi but
    msi.get_image() """
    return img.reshape(-1, img.shape[-1])


def remove_masked_elements(img):
    """ helper method which removes masked pixels.
    Note that by applying this method, the img loses it's shape."""
    collapsed_image = collapse_image(img)
    # if one reflectance is masked msis are defined to have all refl.
    # masked. Thus we can just have a look at the first column
    one_column = collapsed_image[:, 0]
    masked_elems = np.where(one_column.mask)
    collapsed_image = np.delete(collapsed_image, masked_elems, 0)
    return collapsed_image


def select_n_reflectances(img, n):
    """ randomly select n reflectances from image.
    The output is of shape n x nr_wavelengths """
    collapsed_image = collapse_image(img)
    perms = np.random.permutation(collapsed_image.shape[0])
    first_n_perms = perms[0:n]
    return collapsed_image[first_n_perms, :]
