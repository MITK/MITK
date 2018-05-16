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

#include <mitkPhotoacousticMotionCorrectionFilter.h>

class mitkPhotoacousticMotionCorrectionFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticMotionCorrectionFilterTestSuite);
  MITK_TEST(testSomething);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PhotoacousticMotionCorrectionFilter::Pointer filter;
  mitk::Image::Pointer image;

public:
  void setUp() override {
    // get the filter I need
    filter = mitk::PhotoacousticMotionCorrectionFilter::New();
    // get a 3d mitk image
    image = mitk::Image::New();
    unsigned int * dimensions = new unsigned int[3] {2, 2, 2,};
    mitk::PixelType pt = mitk::MakeScalarPixelType<float>();
    image->Initialize(pt, 3, dimensions);
    delete[] dimensions;
    double * data = new double[8];
    image->SetVolume(data);
    delete[] data;
  }

  void tearDown() override {
  }

  void testSomething() {
    CPPUNIT_ASSERT(true);
  }

  void testSettingFirstInput() {
    
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticMotionCorrectionFilter)
