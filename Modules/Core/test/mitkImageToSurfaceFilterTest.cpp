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
#include "mitkImageToSurfaceFilter.h"
#include "mitkException.h"
#include "mitkTestFixture.h"

#include <mitkIOUtil.h>

bool CompareSurfacePointPositions(mitk::Surface::Pointer s1, mitk::Surface::Pointer s2)
{
  vtkPoints* p1 = s1->GetVtkPolyData()->GetPoints();
  vtkPoints* p2 = s2->GetVtkPolyData()->GetPoints();

  if(p1->GetNumberOfPoints() != p2->GetNumberOfPoints())
    return false;

  for(int i = 0; i < p1->GetNumberOfPoints(); ++i)
  {
    if(p1->GetPoint(i)[0] != p2->GetPoint(i)[0] ||
      p1->GetPoint(i)[1] != p2->GetPoint(i)[1] ||
      p1->GetPoint(i)[2] != p2->GetPoint(i)[2] )
    {
      return true;
    }
  }
  return false;
}

class mitkImageToSurfaceFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageToSurfaceFilterTestSuite);
  MITK_TEST(testImageToSurfaceFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testSurfaceGeneration);
  MITK_TEST(testDecimatePromeshDecimation);
  MITK_TEST(testQuadricDecimation);
  MITK_TEST(testSmoothingOfSurface);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Image::Pointer m_BallImage;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    m_BallImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("BallBinary30x30x30.nrrd"));
  }

  void tearDown()
  {
  }


  void testImageToSurfaceFilterInitialization()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testObject.IsNotNull());

    // testing initialization of member variables!
    CPPUNIT_ASSERT_MESSAGE("Testing initialization of threshold member variable",testObject->GetThreshold() == 1.0f);
    CPPUNIT_ASSERT_MESSAGE("Testing initialization of smooth member variable", testObject->GetSmooth() == false);
    CPPUNIT_ASSERT_MESSAGE("Testing initialization of decimate member variable", testObject->GetDecimate() == mitk::ImageToSurfaceFilter::NoDecimation);
    CPPUNIT_ASSERT_MESSAGE("Testing initialization of target reduction member variable", testObject->GetTargetReduction() == 0.95f);
  }

  void testInput()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    testObject->SetInput(m_BallImage);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testObject->GetInput() == m_BallImage);
  }

  void testSurfaceGeneration()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    testObject->SetInput(m_BallImage);
    testObject->Update();
    mitk::Surface::Pointer resultSurface = NULL;
    resultSurface = testObject->GetOutput();
    CPPUNIT_ASSERT_MESSAGE("Testing surface generation!", testObject->GetOutput() != NULL);
  }

  void testDecimatePromeshDecimation()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    testObject->SetInput(m_BallImage);
    testObject->Update();
    mitk::Surface::Pointer resultSurface = NULL;
    resultSurface = testObject->GetOutput();

    mitk::Surface::Pointer testSurface1 = testObject->GetOutput()->Clone();

    testObject->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);
    testObject->SetTargetReduction(0.5f);
    testObject->Update();
    mitk::Surface::Pointer testSurface2 = testObject->GetOutput()->Clone();

    CPPUNIT_ASSERT_MESSAGE("Testing DecimatePro mesh decimation!", testSurface1->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() > testSurface2->GetVtkPolyData()->GetPoints()->GetNumberOfPoints());
  }

  void testQuadricDecimation()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    testObject->SetInput(m_BallImage);
    testObject->Update();
    mitk::Surface::Pointer resultSurface = NULL;
    resultSurface = testObject->GetOutput();

    mitk::Surface::Pointer testSurface1 = testObject->GetOutput()->Clone();

    testObject->SetDecimate(mitk::ImageToSurfaceFilter::QuadricDecimation);
    testObject->SetTargetReduction(0.5f);
    testObject->Update();
    mitk::Surface::Pointer testSurface3 = testObject->GetOutput()->Clone();

    CPPUNIT_ASSERT_MESSAGE("Testing QuadricDecimation mesh decimation!", testSurface1->GetVtkPolyData()->GetPoints()->GetNumberOfPoints() > testSurface3->GetVtkPolyData()->GetPoints()->GetNumberOfPoints());

  }

  void testSmoothingOfSurface()
  {
    mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
    testObject->SetInput(m_BallImage);
    testObject->Update();
    mitk::Surface::Pointer resultSurface = NULL;
    resultSurface = testObject->GetOutput();

    mitk::Surface::Pointer testSurface1 = testObject->GetOutput()->Clone();

    testObject->SetSmooth(true);
    testObject->SetDecimate(mitk::ImageToSurfaceFilter::NoDecimation);
    testObject->Update();
    mitk::Surface::Pointer testSurface4 = testObject->GetOutput()->Clone();
    CPPUNIT_ASSERT_MESSAGE("Testing smoothing of surface changes point data!", CompareSurfacePointPositions(testSurface1, testSurface4));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkImageToSurfaceFilter)
