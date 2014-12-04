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
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>
#include <mitkTestFixture.h>
#include <mitkUnstructuredGrid.h>

#include <mitkIOUtil.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

class mitkUnstructuredGridToUnstructuredGridFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUnstructuredGridToUnstructuredGridFilterTestSuite);
  MITK_TEST(testImageToUnstructuredGridFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testUnstructuredGridGeneration);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::UnstructuredGrid::Pointer m_UnstructuredGrid;

public:

  void setUp()
  {
    m_UnstructuredGrid = mitk::UnstructuredGrid::New();
    vtkSmartPointer<vtkUnstructuredGrid> vtkGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    for(int i=0; i<3; i++)
    {
      for(int j=0; j<3; j++)
      {
        for(int k=0; k<3; k++)
        {
          mitk::Point3D point;
          point[0]=i;
          point[1]=j;
          point[2]=k;

          points->InsertNextPoint(point[0],point[1],point[2]);
        }
      }
    }

    vtkGrid->SetPoints(points);
    m_UnstructuredGrid->SetVtkUnstructuredGrid(vtkGrid);
  }

  void testImageToUnstructuredGridFilterInitialization()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter = mitk::UnstructuredGridToUnstructuredGridFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
  }

  void testInput()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter = mitk::UnstructuredGridToUnstructuredGridFilter::New();
    testFilter->SetInput(m_UnstructuredGrid);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", testFilter->GetInput() == m_UnstructuredGrid);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::UnstructuredGridToUnstructuredGridFilter::Pointer testFilter = mitk::UnstructuredGridToUnstructuredGridFilter::New();
    testFilter->SetInput(m_UnstructuredGrid);
    testFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing surface generation!", testFilter->GetOutput() != NULL);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkUnstructuredGridToUnstructuredGridFilter)
