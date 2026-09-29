/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkReduceContourSetFilter.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>

#include <cmath>

class mitkReduceContourSetFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkReduceContourSetFilterTestSuite);
  MITK_TEST(TestReduceContourWithNthPoint);
  MITK_TEST(TestReduceContourWithDouglasPeuker);
  MITK_TEST(TestReduceContourCutsLongSegmentsWithoutDuplicates);
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
    mitk::Surface::Pointer contour =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/SingleContour.vtk"));
    m_ContourReducer->SetInput(contour);
    m_ContourReducer->SetReductionType(mitk::ReduceContourSetFilter::NTH_POINT);
    m_ContourReducer->SetStepSize(20);
    m_ContourReducer->Update();
    mitk::Surface::Pointer reducedContour = m_ContourReducer->GetOutput();

    mitk::Surface::Pointer reference =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/ReducedContourNthPoint_20.vtk"));

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(reducedContour->GetVtkPolyData()), *(reference->GetVtkPolyData()), 0.000001, true));
  }

  // Reduce contours with Douglas Peucker
  void TestReduceContourWithDouglasPeuker()
  {
    mitk::Surface::Pointer contour =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/TwoContours.vtk"));
    m_ContourReducer->SetInput(contour);
    m_ContourReducer->SetReductionType(mitk::ReduceContourSetFilter::DOUGLAS_PEUCKER);
    m_ContourReducer->Update();
    mitk::Surface::Pointer reducedContour = m_ContourReducer->GetOutput();

    mitk::Surface::Pointer reference =
      mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("SurfaceInterpolation/Reference/ReducedContourDouglasPeucker.vtk"));

    CPPUNIT_ASSERT_MESSAGE(
      "Unequal contours",
      mitk::Equal(*(reducedContour->GetVtkPolyData()), *(reference->GetVtkPolyData()), 0.000001, true));
  }

  // Segments longer than 25 points are cut into equal parts, without a cut point on or next to their end
  void TestReduceContourCutsLongSegmentsWithoutDuplicates()
  {
    // A square with sides of 40 points, one unit apart, closed by repeating the first point
    // like contours of segmentations. Douglas-Peucker keeps the corners, and each side is
    // then cut once in the middle.
    const double corners[4][2] = { { 0, 0 }, { 40, 0 }, { 40, 40 }, { 0, 40 } };
    const double steps[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

    auto points = vtkSmartPointer<vtkPoints>::New();
    auto polygon = vtkSmartPointer<vtkPolygon>::New();

    for (int side = 0; side < 4; ++side)
    {
      for (int i = 0; i < 40; ++i)
      {
        const auto id = points->InsertNextPoint(
          corners[side][0] + i * steps[side][0], corners[side][1] + i * steps[side][1], 0.0);
        polygon->GetPointIds()->InsertNextId(id);
      }
    }

    polygon->GetPointIds()->InsertNextId(points->InsertNextPoint(0.0, 0.0, 0.0));

    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    auto contour = mitk::Surface::New();
    contour->SetVtkPolyData(polyData);

    m_ContourReducer->SetInput(contour);
    m_ContourReducer->SetReductionType(mitk::ReduceContourSetFilter::DOUGLAS_PEUCKER);
    m_ContourReducer->Update();

    auto* reducedContour = m_ContourReducer->GetOutput()->GetVtkPolyData();

    CPPUNIT_ASSERT_EQUAL(vtkIdType(1), reducedContour->GetNumberOfPolys());

    // The corners and the middles of the sides, then the repeated first point
    CPPUNIT_ASSERT_EQUAL(vtkIdType(9), reducedContour->GetNumberOfPoints());

    for (vtkIdType i = 1; i < reducedContour->GetNumberOfPoints(); ++i)
    {
      double previous[3];
      double current[3];
      reducedContour->GetPoint(i - 1, previous);
      reducedContour->GetPoint(i, current);

      CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, std::sqrt(vtkMath::Distance2BetweenPoints(previous, current)), 1e-6);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkReduceContourSetFilter)
