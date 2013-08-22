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

#include "mitkSurface.h"
#include "mitkTestingMacros.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolygon.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>

mitk::Surface::Pointer m_Surface3D;
mitk::Surface::Pointer m_Surface3DLine;
mitk::Surface::Pointer m_Surface3DTwoTimeSteps;
mitk::Surface::Pointer m_Surface3DTwoTimeStepsDifferentPoints;

/**
 * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the two used members for a new test case.
 */
void Setup()
{
  //generate two sets of points
  vtkSmartPointer<vtkPoints> pointsOne = vtkSmartPointer<vtkPoints>::New();
  pointsOne->InsertNextPoint( 0.0, 0.0, 0.0 );
  pointsOne->InsertNextPoint( 1.0, 0.0, 0.0 );
  pointsOne->InsertNextPoint( 0.0, 1.0, 0.0 );
  pointsOne->InsertNextPoint( 1.0, 1.0, 0.0 );

  vtkSmartPointer<vtkPoints> pointsTwo = vtkSmartPointer<vtkPoints>::New();
  pointsTwo->InsertNextPoint( 0.0, 0.0, 0.0 );
  pointsTwo->InsertNextPoint( 0.0, 0.0, 2.0 );
  pointsTwo->InsertNextPoint( 0.0, 1.0, 0.0 );
  pointsTwo->InsertNextPoint( 0.0, 1.0, 2.0 );

  //generate two polygons
  vtkSmartPointer<vtkPolygon> polygonOne = vtkSmartPointer<vtkPolygon>::New();
  polygonOne->GetPointIds()->SetNumberOfIds(4);
  polygonOne->GetPointIds()->SetId(0,0);
  polygonOne->GetPointIds()->SetId(1,1);
  polygonOne->GetPointIds()->SetId(2,2);
  polygonOne->GetPointIds()->SetId(3,3);

  vtkSmartPointer<vtkPolygon> polygonTwo = vtkSmartPointer<vtkPolygon>::New();
  polygonTwo->GetPointIds()->SetNumberOfIds(4);
  polygonTwo->GetPointIds()->SetId(0,3);
  polygonTwo->GetPointIds()->SetId(1,2);
  polygonTwo->GetPointIds()->SetId(2,0);
  polygonTwo->GetPointIds()->SetId(3,1);

  //generate a line
  vtkSmartPointer<vtkPolyLine> polyLineOne = vtkSmartPointer<vtkPolyLine>::New();
  polyLineOne->GetPointIds()->SetNumberOfIds(2);
  polyLineOne->GetPointIds()->SetId(0,0);
  polyLineOne->GetPointIds()->SetId(1,1);

  //generate polydatas
  vtkSmartPointer<vtkCellArray> polygonArrayOne = vtkSmartPointer<vtkCellArray>::New();
  polygonArrayOne->InsertNextCell(polygonOne);

  vtkSmartPointer<vtkPolyData> polyDataOne = vtkSmartPointer<vtkPolyData>::New();
  polyDataOne->SetPoints(pointsOne);
  polyDataOne->SetPolys(polygonArrayOne);

  vtkSmartPointer<vtkCellArray> polygonArrayTwo = vtkSmartPointer<vtkCellArray>::New();
  polygonArrayTwo->InsertNextCell(polygonTwo);

  vtkSmartPointer<vtkPolyData> polyDataTwo = vtkSmartPointer<vtkPolyData>::New();
  polyDataTwo->SetPoints(pointsOne);
  polyDataTwo->SetPolys(polygonArrayTwo);

  vtkSmartPointer<vtkPolyData> polyDataThree = vtkSmartPointer<vtkPolyData>::New();
  polyDataThree->SetPoints(pointsTwo);
  polyDataThree->SetPolys(polygonArrayTwo);

  vtkSmartPointer<vtkCellArray> polyLineArrayOne = vtkSmartPointer<vtkCellArray>::New();
  polyLineArrayOne->InsertNextCell(polyLineOne);

  vtkSmartPointer<vtkPolyData> polyDataFour = vtkSmartPointer<vtkPolyData>::New();
  polyDataFour->SetPoints(pointsOne);
  polyDataFour->SetLines(polyLineArrayOne);

  //generate surfaces
  m_Surface3D = mitk::Surface::New();
  m_Surface3D->SetVtkPolyData( polyDataOne );

  m_Surface3DTwoTimeSteps = mitk::Surface::New();
  m_Surface3DTwoTimeSteps->SetVtkPolyData( polyDataOne, 0 );
  m_Surface3DTwoTimeSteps->SetVtkPolyData( polyDataTwo, 1 );

  m_Surface3DTwoTimeStepsDifferentPoints = mitk::Surface::New();
  m_Surface3DTwoTimeStepsDifferentPoints->SetVtkPolyData( polyDataOne, 0 );
  m_Surface3DTwoTimeStepsDifferentPoints->SetVtkPolyData( polyDataThree, 1 );

  m_Surface3DLine = mitk::Surface::New();
  m_Surface3DLine->SetVtkPolyData( polyDataFour );
}

void Equal_CloneAndOriginalOneTimestep_ReturnsTrue()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( mitk::Equal( m_Surface3D, m_Surface3D->Clone() ), "A one timestep clone should be equal to its original.\n");
}

void Equal_CloneAndOriginalTwoTimesteps_ReturnsTrue()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( mitk::Equal( m_Surface3DTwoTimeSteps, m_Surface3DTwoTimeSteps->Clone() ), "A one timestep clone should be equal to its original.\n");
}

void Equal_OneTimeStepVSTwoTimeStep_ReturnsFalse()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( ! mitk::Equal( m_Surface3D, m_Surface3DTwoTimeSteps ), "A one timestep and two timestep surface should not be equal.\n");
}

void Equal_TwoTimeStepsDifferentPoints_ReturnsFalse()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( ! mitk::Equal( m_Surface3DTwoTimeStepsDifferentPoints, m_Surface3DTwoTimeSteps ), "A surface with the same timesteps and different points should not be equal.\n");
}

void Equal_SurfaceWithPolygonSurfaceWithPolyLine_ReturnsFalse()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( ! mitk::Equal( m_Surface3D, m_Surface3DLine ), "A surface with the same timesteps and points and the same number of cells, but different types of cells should not be equal.\n");
}

/**
 * @brief mitkSurfaceEqualTest A test class for Equal methods in mitk::Surface.
 */
int mitkSurfaceEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkSurfaceEqualTest);

  Equal_CloneAndOriginalOneTimestep_ReturnsTrue();
  Equal_CloneAndOriginalTwoTimesteps_ReturnsTrue();
  Equal_OneTimeStepVSTwoTimeStep_ReturnsFalse();
  Equal_TwoTimeStepsDifferentPoints_ReturnsFalse();
  Equal_SurfaceWithPolygonSurfaceWithPolyLine_ReturnsFalse();

  MITK_TEST_END();

  return EXIT_SUCCESS;
}
