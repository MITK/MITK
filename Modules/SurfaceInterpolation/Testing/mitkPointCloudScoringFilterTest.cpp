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
#include <mitkPointCloudScoringFilter.h>
#include <mitkTestFixture.h>
#include <mitkUnstructuredGrid.h>

#include <mitkIOUtil.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

class mitkPointCloudScoringFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPointCloudScoringFilterTestSuite);
  MITK_TEST(testPointCloudScoringFilterInitialization);
  MITK_TEST(testInputs);
  MITK_TEST(testOutput);
  MITK_TEST(testScoring);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::UnstructuredGrid::Pointer m_UnstructuredGrid1;
  mitk::UnstructuredGrid::Pointer m_UnstructuredGrid2;

public:

  void setUp() override
  {
    m_UnstructuredGrid1 = mitk::UnstructuredGrid::New();
    m_UnstructuredGrid2 = mitk::UnstructuredGrid::New();

    vtkSmartPointer<vtkUnstructuredGrid> vtkGrid1 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkSmartPointer<vtkUnstructuredGrid> vtkGrid2 = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkSmartPointer<vtkPoints> points1 = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> points2 = vtkSmartPointer<vtkPoints>::New();

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

          points1->InsertNextPoint(point[0],point[1],point[2]);
          points2->InsertNextPoint(point[0]+i,point[1]+i,point[2]+i);
        }
      }
    }

    vtkGrid1->SetPoints(points1);
    vtkGrid2->SetPoints(points2);
    m_UnstructuredGrid1->SetVtkUnstructuredGrid(vtkGrid1);
    m_UnstructuredGrid2->SetVtkUnstructuredGrid(vtkGrid2);
  }

  void testPointCloudScoringFilterInitialization()
  {
    mitk::PointCloudScoringFilter::Pointer testFilter = mitk::PointCloudScoringFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of test object", testFilter.IsNotNull());
  }

  void testInputs()
  {
    mitk::PointCloudScoringFilter::Pointer testFilter = mitk::PointCloudScoringFilter::New();
    testFilter->SetInput(0, m_UnstructuredGrid1);
    testFilter->SetInput(1, m_UnstructuredGrid2);

    mitk::UnstructuredGrid::Pointer uGrid1 = dynamic_cast<mitk::UnstructuredGrid*>(testFilter->GetInputs().at(0).GetPointer());
    mitk::UnstructuredGrid::Pointer uGrid2 = dynamic_cast<mitk::UnstructuredGrid*>(testFilter->GetInputs().at(1).GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Testing the first input", uGrid1 == m_UnstructuredGrid1);
    CPPUNIT_ASSERT_MESSAGE("Testing the second input", uGrid2 == m_UnstructuredGrid2);
  }

  void testOutput()
  {
    mitk::PointCloudScoringFilter::Pointer testFilter = mitk::PointCloudScoringFilter::New();
    testFilter->SetInput(0, m_UnstructuredGrid1);
    testFilter->SetInput(1, m_UnstructuredGrid2);
    testFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing mitkUnstructuredGrid generation!", testFilter->GetOutput() != NULL);
  }

  void testScoring()
  {
    mitk::PointCloudScoringFilter::Pointer testFilter = mitk::PointCloudScoringFilter::New();
    testFilter->SetInput(0, m_UnstructuredGrid2);
    testFilter->SetInput(1, m_UnstructuredGrid1);
    testFilter->Update();

    mitk::UnstructuredGrid::Pointer outpGrid = testFilter->GetOutput();

    int numBefore = m_UnstructuredGrid1->GetVtkUnstructuredGrid()->GetNumberOfPoints();
    int numAfter = outpGrid->GetVtkUnstructuredGrid()->GetNumberOfPoints();

    CPPUNIT_ASSERT_MESSAGE("Testing grid scoring", numBefore > numAfter);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPointCloudScoringFilter)
