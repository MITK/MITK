
import unittest
import os

from msi.io.tiffwriter import TiffWriter
from msi.io.tiffreader import TiffReader
from msi.test import helpers


class TestTiffWriter(unittest.TestCase):

    def setUp(self):
        # setup file and the path where it shall be written to
        self.msi = helpers.getFakeMsi()
        self.msi.set_image(self.msi.get_image())
        self.fileUriToWrite = os.path.join(os.getcwd(), "testfiles")

    def tearDown(self):
        # remove the hopefully written files
        folder, file_prefix = os.path.split(self.fileUriToWrite)
        image_files = [f for f in os.listdir(folder) if
                       os.path.isfile(os.path.join(folder, f))]
        image_files = [f for f in image_files if f.startswith(file_prefix)]
        # expand to full path
        image_files = [os.path.join(folder, f) for f in image_files]
        for f in image_files:
            os.remove(f)

    def test_imageWriterCreatesFile(self):
        writer = TiffWriter(self.msi, convert_to_nm=False)
        writer.write(self.fileUriToWrite)

    def test_imageWriterCreatesCorrectFile(self):
        writer = TiffWriter(self.msi, convert_to_nm=False)
        writer.write(self.fileUriToWrite)

        reader = TiffReader(shift_bits=0)
        msi = reader.read(self.fileUriToWrite)
        self.assertTrue(msi == helpers.getFakeMsi(),
                                       "image correctly written and read")
