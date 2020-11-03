/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include <mitkTestFixture.h>
#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>

#include <mitkIOUtil.h>

#include <vtkDebugLeaks.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

class mitkUnstructuredGridToUnstructuredGridFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUnstructuredGridToUnstructuredGridFilterTestSuite);

  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(testUnstructuredGridToUnstructuredGridFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testMultipleInputs);
  MITK_TEST(testUnstructuredGridGeneration);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::UnstructuredGrid::Pointer m_UnstructuredGrid;
  mitk::UnstructuredGrid::Pointer m_2ndUnstructuredGrid;

public:
  void setUp() override
  {
    m_UnstructuredGrid = mitk::UnstructuredGrid::New();
    m_2ndUnstructuredGrid = mitk::UnstructuredGrid::New();
    vtkSmartPointer<vtkUnstructuredGrid> vtkGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkSmartPointer<vtkUnstructuredGrid> vtkGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> points2 = vtkSmartPointer<vtkPoints>::New();

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        for (int k = 0; k < 3; k++)
        {
          mitk::Point3D point;
          point[0] = i;
          point[1] = j;
          point[2] = k;

          points->InsertNextPoint(point[0], point[1], point[2]);
          points2->InsertNextPoint(point[0] + 5, point[1] + 5, point[2] + 5);
        }
      }
    }

    vtkGrid->SetPoints(points);
    vtkGrid2->SetPoints(points2);
    m_UnstructuredGrid->SetVtkUnstructuredGrid(vtkGrid);
    m_2ndUnstructuredGrid->SetVtkUnstructuredGrid(vtkGrid2);
  }

  void testUnstructuredGridToUnstructuredGridFilterInitialization()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter =
      mitk::UnstructuredGridToUnstructuredGridFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
  }

  void testInput()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter =
      mitk::UnstructuredGridToUnstructuredGridFilter::New();
    testFilter->SetInput(m_UnstructuredGrid);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testFilter->GetInput() == m_UnstructuredGrid);
  }

  void testMultipleInputs()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter =
      mitk::UnstructuredGridToUnstructuredGridFilter::New();
    testFilter->SetInput(0, m_UnstructuredGrid);
    testFilter->SetInput(1, m_2ndUnstructuredGrid);
    CPPUNIT_ASSERT_MESSAGE("Testing first input!", testFilter->GetInput(0) == m_UnstructuredGrid);
    CPPUNIT_ASSERT_MESSAGE("Testing second input!", testFilter->GetInput(1) == m_2ndUnstructuredGrid);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter =
      mitk::UnstructuredGridToUnstructuredGridFilter::New();
    testFilter->SetInput(m_UnstructuredGrid);
    testFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing unstructured grid generation!", testFilter->GetOutput() != nullptr);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkUnstructuredGridToUnstructuredGridFilter)
