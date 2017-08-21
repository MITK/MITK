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

#include "mitkTestingMacros.h"
#include <mitkImageToPointCloudFilter.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>

class mitkImageToPointCloudFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageToPointCloudFilterTestSuite);
  MITK_TEST(testImageToPointCloudFilterInitialization);
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
  void setUp() { m_BallImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("BallBinary30x30x30.nrrd"))[0].GetPointer()); }
  void testImageToPointCloudFilterInitialization()
  {
    mitk::ImageToUnstructuredGridFilter::Pointer testFilter = mitk::ImageToUnstructuredGridFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
  }

  void testInput()
  {
    mitk::ImageToPointCloudFilter::Pointer testFilter = mitk::ImageToPointCloudFilter::New();
    testFilter->SetInput(m_BallImage);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testFilter->GetInput() == m_BallImage);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::ImageToPointCloudFilter::Pointer testFilter = mitk::ImageToPointCloudFilter::New();
    testFilter->SetInput(m_BallImage);
    testFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing UnstructuredGrid generation!", testFilter->GetOutput() != nullptr);
  }

  void testThreshold()
  {
    mitk::ImageToPointCloudFilter::Pointer testFilter1 = mitk::ImageToPointCloudFilter::New();
    testFilter1->SetInput(m_BallImage);
    testFilter1->Update();

    int numberOfPoints1 = testFilter1->GetNumberOfExtractedPoints();

    mitk::ImageToPointCloudFilter::Pointer testFilter2 = mitk::ImageToPointCloudFilter::New();
    testFilter2->SetInput(m_BallImage);
    testFilter2->SetMethod(mitk::ImageToPointCloudFilter::LAPLACIAN_STD_DEV3);
    testFilter2->Update();

    int numberOfPoints2 = testFilter2->GetNumberOfExtractedPoints();

    CPPUNIT_ASSERT_MESSAGE("Testing Threshold", numberOfPoints1 > numberOfPoints2);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageToPointCloudFilter)
