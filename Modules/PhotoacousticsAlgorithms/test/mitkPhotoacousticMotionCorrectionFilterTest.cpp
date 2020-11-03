/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPhotoacousticMotionCorrectionFilter.h>

class mitkPhotoacousticMotionCorrectionFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticMotionCorrectionFilterTestSuite);
  MITK_TEST(testSettingFirstInput);
  MITK_TEST(testSettingSecondInput);
  // MITK_TEST(testNoThirdInput);
  MITK_TEST(testGetFirstEmptyOutput);
  MITK_TEST(testGetSecondEmptyOutput);
  MITK_TEST(testSameInOutDimensions);
  MITK_TEST(testNo3DError);
  MITK_TEST(testSameInputDimension1);
  MITK_TEST(testSameInputDimension2);
  MITK_TEST(testNullPtr1);
  MITK_TEST(testNullPtr2);
  MITK_TEST(testNullPtr3);
  MITK_TEST(testSameInputDimensions);
  MITK_TEST(testStaticSliceCorrection);
  CPPUNIT_TEST_SUITE_END();


private:
  mitk::PhotoacousticMotionCorrectionFilter::Pointer filter;
  mitk::Image::Pointer image, image4d, image2d, image3d1slice;
  float * data2d;

public:
  void setUp() override {
    // get the filter I need
    filter = mitk::PhotoacousticMotionCorrectionFilter::New();
    // get a 3d mitk image
    image = mitk::Image::New();
    image2d = mitk::Image::New();
    image4d = mitk::Image::New();
    image3d1slice = mitk::Image::New();
    unsigned int * dimensions = new unsigned int[3] {2, 2, 2};
    unsigned int * dimensions2 = new unsigned int[3] {2, 2, 1};
    unsigned int * dim2 = new unsigned int[4] {2, 2, 2, 2};
    mitk::PixelType pt = mitk::MakeScalarPixelType<float>();
    float * data = new float[8] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float * data4d = new float[16];
    data2d = new float[4] {500.0, -500.5, 1000.0, 0.5};
    image->Initialize(pt, 3, dimensions);
    image->SetVolume(data);
    image4d->Initialize(pt, 4, dim2);
    image4d->SetVolume(data4d);
    image2d->Initialize(pt, 2, dimensions);
    image2d->SetVolume(data2d);
    image3d1slice->Initialize(pt, 3, dimensions2);
    image3d1slice->SetVolume(data);
    delete[] dimensions;
    delete[] dimensions2;
    delete[] data;
    delete[] data4d;
    delete[] dim2;
  }

  void tearDown() override {
    delete[] data2d;
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

  // void testNoThirdInput() {
  //   CPPUNIT_ASSERT_NO_THROW(filter->SetInput(2, image));
  //   // CPPUNIT_ASSERT(true);
  // }

  void testGetFirstEmptyOutput() {
    mitk::Image::Pointer out = filter->GetOutput(0);
    unsigned int dim = 0;
    CPPUNIT_ASSERT_EQUAL(dim, out->GetDimension());
  }

  void testGetSecondEmptyOutput() {
    mitk::Image::Pointer out = filter->GetOutput(1);
    unsigned int dim = 0;
    CPPUNIT_ASSERT_EQUAL(dim, out->GetDimension());
  }

  void testSameInOutDimensions() {
    filter->SetInput(0, image);
    filter->SetInput(1, image);
    filter->Update();
    mitk::Image::Pointer out = filter->GetOutput(0);
    CPPUNIT_ASSERT_EQUAL(image->GetDimension(), out->GetDimension());
    out = filter->GetOutput(1);
    CPPUNIT_ASSERT_EQUAL(image->GetDimension(), out->GetDimension());
  }

  void testNo3DError() {
    filter->SetInput(0, image4d);
    filter->SetInput(1, image4d);
    CPPUNIT_ASSERT_THROW(filter->Update(), std::invalid_argument);
  }

  // I only test for dim input0 <= dim input1, because otherwise
  // itk will throw an error before the program even reaches the
  // parts I wrote.
  // Same for testSameInputDimension2
  void testSameInputDimension1() {
    filter->SetInput(0, image);
    filter->SetInput(1, image4d);
    CPPUNIT_ASSERT_THROW(filter->Update(), std::invalid_argument);
  }

  // See previous comment
  void testSameInputDimension2() {
    filter->SetInput(0, image2d);
    filter->SetInput(1, image);
    CPPUNIT_ASSERT_THROW(filter->Update(), std::invalid_argument);
  }

  // I tried to catch the error myself in the filter, but itk does some magic beforehand
  void testNullPtr1() {
    filter->SetInput(0, nullptr);
    filter->SetInput(1, image);
    CPPUNIT_ASSERT_THROW(filter->Update(), itk::ExceptionObject);
  }

  // Now I am allowed to catch it myself, because the first input is fine -.-
  void testNullPtr2() {
    filter->SetInput(0, image);
    filter->SetInput(1, nullptr);
    CPPUNIT_ASSERT_THROW(filter->Update(), std::invalid_argument);
  }

  void testNullPtr3() {
    filter->SetInput(0, nullptr);
    filter->SetInput(1, nullptr);
    CPPUNIT_ASSERT_THROW(filter->Update(), itk::ExceptionObject);
  }

  void testSameInputDimensions() {
    filter->SetInput(0, image3d1slice);
    filter->SetInput(1, image);
    CPPUNIT_ASSERT_THROW(filter->Update(), std::invalid_argument);
  }

  void testStaticSliceCorrection() {
    image->SetSlice(data2d, 0);
    image->SetSlice(data2d, 1);
    filter->SetInput(0, image);
    filter->SetInput(1, image);
    filter->Update();
    mitk::Image::Pointer out1 = filter->GetOutput(0);
    mitk::Image::Pointer out2 = filter->GetOutput(1);
    MITK_ASSERT_EQUAL(image, out1, "Check that static image does not get changed.");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticMotionCorrectionFilter)
