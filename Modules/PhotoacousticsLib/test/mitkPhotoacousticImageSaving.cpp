/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImage.h>
#include <mitkIOUtil.h>

class mitkPhotoacousticImageSavingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticImageSavingTestSuite);
  MITK_TEST(testSaveImagePlainMitk);
  CPPUNIT_TEST_SUITE_END();

private:

public:

  void setUp()
  {
  }

  void testSaveImagePlainMitk()
  {
    unsigned int x = 560;
    unsigned int y = 400;
    unsigned int z = 720;
    unsigned int t = 4;

    unsigned long size = x*y*z*t;

    double* data = new double[size];

    for (unsigned long i = 0; i < size; i++)
    {
      data[i] = rand() * 9.7327;
    }

    mitk::Image::Pointer image = mitk::Image::New();
    mitk::PixelType TPixel = mitk::MakeScalarPixelType<double>();
    unsigned int* dimensionsOfImage = new unsigned int[4];

    // Copy dimensions
    dimensionsOfImage[0] = 560;
    dimensionsOfImage[1] = 400;
    dimensionsOfImage[2] = 720;
    dimensionsOfImage[3] = 4;

    image->Initialize(TPixel, 4, dimensionsOfImage, 1);

    mitk::Vector3D spacing;
    spacing.Fill(0.1);
    image->SetSpacing(spacing);

    image->SetImportVolume(data, 0, 0, mitk::Image::CopyMemory);
    image->SetImportVolume(data, 1, 0, mitk::Image::CopyMemory);
    image->SetImportVolume(data, 2, 0, mitk::Image::CopyMemory);
    image->SetImportVolume(data, 3, 0, mitk::Image::CopyMemory);

    mitk::IOUtil::Save(image, "C:/Users/groehl/Desktop/test.nrrd");
  }

  void tearDown()
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticImageSaving)
