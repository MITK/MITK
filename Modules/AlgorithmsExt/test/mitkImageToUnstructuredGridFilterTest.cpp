/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include <mitkImageToUnstructuredGridFilter.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>

class mitkImageToUnstructuredGridFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageToUnstructuredGridFilterTestSuite);
  MITK_TEST(testImageToUnstructuredGridFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testUnstructuredGridGeneration);
  MITK_TEST(testThreshold);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Image::Pointer m_BallImage;

public:
  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
   * members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override { m_BallImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("BallBinary30x30x30.nrrd")); }
  void testImageToUnstructuredGridFilterInitialization()
  {
    mitk::ImageToUnstructuredGridFilter::Pointer testFilter = mitk::ImageToUnstructuredGridFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Testing initialization of threshold member variable", testFilter->GetThreshold() == -0.1);
  }

  void testInput()
  {
    mitk::ImageToUnstructuredGridFilter::Pointer testFilter = mitk::ImageToUnstructuredGridFilter::New();
    testFilter->SetInput(m_BallImage);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testFilter->GetInput() == m_BallImage);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::ImageToUnstructuredGridFilter::Pointer testFilter = mitk::ImageToUnstructuredGridFilter::New();
    testFilter->SetInput(m_BallImage);
    testFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing UnstructuredGrid generation!", testFilter->GetOutput() != nullptr);
  }

  void testThreshold()
  {
    mitk::ImageToUnstructuredGridFilter::Pointer testFilter1 = mitk::ImageToUnstructuredGridFilter::New();
    testFilter1->SetInput(m_BallImage);
    testFilter1->Update();

    int numberOfPoints1 = testFilter1->GetNumberOfExtractedPoints();

    mitk::ImageToUnstructuredGridFilter::Pointer testFilter2 = mitk::ImageToUnstructuredGridFilter::New();
    testFilter2->SetInput(m_BallImage);
    testFilter2->SetThreshold(1.0);
    testFilter2->Update();

    int numberOfPoints2 = testFilter2->GetNumberOfExtractedPoints();

    CPPUNIT_ASSERT_MESSAGE("Testing Threshold", numberOfPoints1 > numberOfPoints2);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageToUnstructuredGridFilter)
