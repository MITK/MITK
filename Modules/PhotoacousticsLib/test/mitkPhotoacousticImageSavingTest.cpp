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
    unsigned int x = pow(2, 30);
    unsigned long size = x;

    float* data = new float[size];
    for (unsigned long i = 0; i < size; i++)
      data[i] = 0;

    mitk::Image::Pointer image = mitk::Image::New();
    mitk::PixelType TPixel = mitk::MakeScalarPixelType<float>();
    unsigned int* dimensionsOfImage = new unsigned int[1];
    dimensionsOfImage[0] = x;
    image->Initialize(TPixel, 1, dimensionsOfImage, 1);
    image->SetImportVolume(data, 0, 0, mitk::Image::CopyMemory);

    mitk::IOUtil::Save(image, "C:/Users/groehl/Desktop/test.nrrd");
  }

  void tearDown()
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticImageSaving)
