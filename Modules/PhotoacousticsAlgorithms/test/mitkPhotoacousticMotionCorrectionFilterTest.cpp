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
  MITK_TEST(testSettingFirstInput);
  MITK_TEST(testSettingSecondInput);
  MITK_TEST(testNoThirdInput);
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
    double * data = new double[8] {0, 0, 0, 0, 0, 0, 0, 0};
    image->SetVolume(data);
    delete[] data;
  }

  void tearDown() override {
  }

  void testSettingFirstInput() {
    filter->SetInput(0, image);
    mitk::Image::Pointer out = filter->GetInput(0);
    CPPUNIT_ASSERT_EQUAL(image, out);
  }

  void testSettingSecondInput() {
    filter->SetInput(1, image);
    mitk::Image::Pointer out = filter->GetInput(1);
    CPPUNIT_ASSERT_EQUAL(image, out);
  }

  void testNoThirdInput() {
    CPPUNIT_ASSERT_ASSERTION_FAIL(filter->SetInput(2, image));
    // CPPUNIT_ASSERT(true);
  }

  void testGetFirstEmptyOutput() {
    
  }

  void testGetSecondEmptyOutput() {

  }

  void testSameInOutDimensions() {
    
  }

  void testSameInputDimensions() {
    
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticMotionCorrectionFilter)
