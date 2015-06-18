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

#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkReduceContourSetFilter.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkReduceContourSetFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkReduceContourSetFilterTestSuite);
  MITK_TEST(TestReduceContourWithNthPoint);
  MITK_TEST(TestReduceContourWithDouglasPeuker);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ReduceContourSetFilter::Pointer m_ContourReducer;

public:

  void setUp() override
  {
    m_ContourReducer = mitk::ReduceContourSetFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize ReduceContourSetFilter", m_ContourReducer.IsNotNull());
  }

  // Reduce contours with nth point
  void TestReduceContourWithNthPoint()
  {
    mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/SingleContour.vtk"));
    m_ContourReducer->SetInput(contour);
    m_ContourReducer->SetReductionType(mitk::ReduceContourSetFilter::NTH_POINT);
    m_ContourReducer->SetStepSize(20);
    m_ContourReducer->Update();
    mitk::Surface::Pointer reducedContour = m_ContourReducer->GetOutput();

    mitk::Surface::Pointer reference = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/ReducedContourNthPoint_20.vtk"));

    CPPUNIT_ASSERT_MESSAGE("Unequal contours", mitk::Equal(*(reducedContour->GetVtkPolyData()), *(reference->GetVtkPolyData()), 0.000001, true));
  }

  // Reduce contours with Douglas Peucker
  void TestReduceContourWithDouglasPeuker()
  {
    mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/TwoContours.vtk"));
    m_ContourReducer->SetInput(contour);
    m_ContourReducer->SetReductionType(mitk::ReduceContourSetFilter::DOUGLAS_PEUCKER);
    m_ContourReducer->Update();
    mitk::Surface::Pointer reducedContour = m_ContourReducer->GetOutput();

    mitk::Surface::Pointer reference = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceInterpolation/Reference/ReducedContourDouglasPeucker.vtk"));

    CPPUNIT_ASSERT_MESSAGE("Unequal contours", mitk::Equal(*(reducedContour->GetVtkPolyData()), *(reference->GetVtkPolyData()), 0.000001, true));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkReduceContourSetFilter)
