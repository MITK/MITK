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
#include <mitkFeatureBasedEdgeDetectionFilter.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>

class mitkFeatureBasedEdgeDetectionFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFeatureBasedEdgeDetectionFilterTestSuite);
  MITK_TEST(testFeatureBasedEdgeDetectionFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testUnstructuredGridGeneration);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Image::Pointer m_Pic3D;
  mitk::Image::Pointer m_Segmentation;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    m_Pic3D = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    m_Segmentation = mitk::IOUtil::LoadImage(GetTestDataFilePath("PlaneSuggestion/pic3D_segmentation.nrrd"));
  }

  void testFeatureBasedEdgeDetectionFilterInitialization()
  {
    mitk::FeatureBasedEdgeDetectionFilter::Pointer testFilter = mitk::FeatureBasedEdgeDetectionFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
  }

  void testInput()
  {
    mitk::FeatureBasedEdgeDetectionFilter::Pointer testFilter = mitk::FeatureBasedEdgeDetectionFilter::New();
    testFilter->SetInput(m_Pic3D);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testFilter->GetInput() == m_Pic3D);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::FeatureBasedEdgeDetectionFilter::Pointer testFilter = mitk::FeatureBasedEdgeDetectionFilter::New();
    testFilter->SetInput(m_Pic3D);
    testFilter->SetSegmentationMask(m_Segmentation);
    testFilter->Update();

    CPPUNIT_ASSERT_MESSAGE("Testing surface generation!", testFilter->GetOutput()->GetVtkUnstructuredGrid() != NULL);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkFeatureBasedEdgeDetectionFilter)
