

import logging

import numpy as np
from libtiff import TIFF

from msi.io.writer import Writer


class TiffWriter(Writer):
    """
    The tiff write will store nr_wavelength tiff files for one msi
    """

    def __init__(self, msi_to_write, convert_to_nm=True, scale_to_16_bit=False):
        """
        initialize the write with a specific multi spectral image (class Msi)
        """
        self.msi_to_write = msi_to_write
        self.convert_to_nm = convert_to_nm
        self.scale_to_16_bit = scale_to_16_bit

    def write(self, uri_prefix):
        """
        write the msi image to the specified uri_prefix

        Args:
            uri_prefix: the prefix off the uri. E.g. C:\example the image
             write will automatically extend this prefix path to include the
             wavelengths information and add a suffix. Your final
             file may look similar to: C:\example_w_470nm_F0.tiff
            convert_to_nm: if the wavelengths are saved in m they are hard to
             write as string. Thus they can be automatically expanded to nm.
        """
        img_to_write = self.msi_to_write.get_image()

        max_image_value = np.max(img_to_write)

        if self.scale_to_16_bit:
            img_to_write *= 2**16 / max_image_value

        nr_wavelengths = self.msi_to_write.get_wavelengths().size
        for wavelength_index in np.arange(nr_wavelengths):
            full_uri = self._build_full_uri(uri_prefix, wavelength_index)
            self._write_single_image(full_uri,
                                     img_to_write[:, :, wavelength_index])

        logging.info("written file " + full_uri + " to disk")
        return None

    @staticmethod
    def _write_single_image(full_uri, image_array):
        """
        internally used method to write single tiff image
        """
        tiff = TIFF.open(full_uri, mode='w')
        tiff.write_image(image_array.astype('uint16'), write_rgb=False)
        tiff.close()

    def _build_full_uri(self, uri_prefix, wavelength_index):
        """
        Helper method to build full path of one image
        Returns: full uri containing the desired properties.
        """
        wavelength = self.msi_to_write.get_wavelengths()[wavelength_index]
        if self.convert_to_nm:
            wavelength *= 10**9
        full_uri = uri_prefix + "_w_" + str(wavelength) +\
                   "_F" + str(wavelength_index) + ".tiff"

        return full_uri
