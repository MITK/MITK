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

/** Members used inside the different (sub-)tests. All members are initialized via Setup().*/
mitk::Surface::Pointer m_Surface3D;
mitk::Surface::Pointer m_Surface3DTwoTimeSteps;

vtkSmartPointer<vtkPoints> m_PointsOne;
vtkSmartPointer<vtkPoints> m_PointsTwo;
vtkSmartPointer<vtkCellArray> m_PolygonArrayTwo;
vtkSmartPointer<vtkPolyData> m_PolyDataOne;

/**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
static void Setup()
{
  //generate two sets of points
  m_PointsOne = vtkSmartPointer<vtkPoints>::New();
  m_PointsOne->InsertNextPoint( 0.0, 0.0, 0.0 );
  m_PointsOne->InsertNextPoint( 1.0, 0.0, 0.0 );
  m_PointsOne->InsertNextPoint( 0.0, 1.0, 0.0 );
  m_PointsOne->InsertNextPoint( 1.0, 1.0, 0.0 );

  m_PointsTwo = vtkSmartPointer<vtkPoints>::New();
  m_PointsTwo->InsertNextPoint( 0.0, 0.0, 0.0 );
  m_PointsTwo->InsertNextPoint( 0.0, 0.0, 2.0 );
  m_PointsTwo->InsertNextPoint( 0.0, 1.0, 0.0 );
  m_PointsTwo->InsertNextPoint( 0.0, 1.0, 2.0 );

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

  //generate polydatas
  vtkSmartPointer<vtkCellArray> polygonArrayOne = vtkSmartPointer<vtkCellArray>::New();
  polygonArrayOne->InsertNextCell(polygonOne);

  m_PolyDataOne = vtkSmartPointer<vtkPolyData>::New();
  m_PolyDataOne->SetPoints(m_PointsOne);
  m_PolyDataOne->SetPolys(polygonArrayOne);

  m_PolygonArrayTwo = vtkSmartPointer<vtkCellArray>::New();
  m_PolygonArrayTwo->InsertNextCell(polygonTwo);

  vtkSmartPointer<vtkPolyData> polyDataTwo = vtkSmartPointer<vtkPolyData>::New();
  polyDataTwo->SetPoints(m_PointsOne);
  polyDataTwo->SetPolys(m_PolygonArrayTwo);

  //generate surfaces
  m_Surface3D = mitk::Surface::New();
  m_Surface3D->SetVtkPolyData( m_PolyDataOne );

  m_Surface3DTwoTimeSteps = mitk::Surface::New();
  m_Surface3DTwoTimeSteps->SetVtkPolyData( m_PolyDataOne, 0 );
  m_Surface3DTwoTimeSteps->SetVtkPolyData( polyDataTwo, 1 );
}

static void Equal_InputIsNull_ReturnsFalse()
{
  mitk::Surface::Pointer surface = NULL;
  MITK_TEST_NOT_EQUAL( surface, surface, "Input is NULL. Result should be false.");
}

static void Equal_CloneAndOriginalOneTimestep_ReturnsTrue()
{
  Setup();
  MITK_TEST_EQUAL( m_Surface3D, m_Surface3D->Clone(), "A one timestep clone should be equal to its original.");
}

static void Equal_CloneAndOriginalTwoTimesteps_ReturnsTrue()
{
  Setup();
  MITK_TEST_EQUAL( m_Surface3DTwoTimeSteps, m_Surface3DTwoTimeSteps->Clone(), "A two timestep clone should be equal to its original.");
}

static void Equal_OneTimeStepVSTwoTimeStep_ReturnsFalse()
{
  Setup();
  MITK_TEST_NOT_EQUAL( m_Surface3D, m_Surface3DTwoTimeSteps, "A one timestep and two timestep surface should not be equal.");
}

static void Equal_TwoTimeStepsDifferentPoints_ReturnsFalse()
{
  Setup();

  vtkSmartPointer<vtkPolyData> polyDataDifferentPoints = vtkSmartPointer<vtkPolyData>::New();
  polyDataDifferentPoints->SetPoints(m_PointsTwo);
  polyDataDifferentPoints->SetPolys(m_PolygonArrayTwo);

  mitk::Surface::Pointer surface3DTwoTimeStepsDifferentPoints = mitk::Surface::New();
  surface3DTwoTimeStepsDifferentPoints->SetVtkPolyData( m_PolyDataOne, 0 );
  surface3DTwoTimeStepsDifferentPoints->SetVtkPolyData( polyDataDifferentPoints, 1 );

  //The geometry also changes, because the second pointset has a different geometry/extent.
  MITK_TEST_NOT_EQUAL( surface3DTwoTimeStepsDifferentPoints, m_Surface3DTwoTimeSteps, "A surface with the same timesteps and different points should not be equal.");
}

static void Equal_SurfaceWithPolygonSurfaceWithPolyLine_ReturnsFalse()
{
  Setup();

  //generate a line
  vtkSmartPointer<vtkPolyLine> polyLineOne = vtkSmartPointer<vtkPolyLine>::New();
  polyLineOne->GetPointIds()->SetNumberOfIds(2);
  polyLineOne->GetPointIds()->SetId(0,0);
  polyLineOne->GetPointIds()->SetId(1,1);

  vtkSmartPointer<vtkCellArray> polyLineArrayOne = vtkSmartPointer<vtkCellArray>::New();
  polyLineArrayOne->InsertNextCell(polyLineOne);

  vtkSmartPointer<vtkPolyData> polyDataLine = vtkSmartPointer<vtkPolyData>::New();
  polyDataLine->SetPoints(m_PointsOne);
  polyDataLine->SetLines(polyLineArrayOne);

  mitk::Surface::Pointer surface3DLine = mitk::Surface::New();
  surface3DLine->SetVtkPolyData( polyDataLine );

  MITK_TEST_NOT_EQUAL( m_Surface3D, surface3DLine, "A surface with the same timesteps and points and the same number of cells, but different types of cells should not be equal.");
}

static void Equal_DifferentPoints_ReturnsFalse()
{
  Setup();
  mitk::Surface::Pointer surfaceWithADifferentPoint = m_Surface3D->Clone();
  //modify points. m_Surface3D contains m_PointsOne
  surfaceWithADifferentPoint->GetVtkPolyData()->SetPoints( m_PointsTwo );

  MITK_TEST_NOT_EQUAL( m_Surface3D, surfaceWithADifferentPoint, "A surface with a single timestep and different points should not be equal.");
}

/**
* @brief mitkSurfaceEqualTest A test class for Equal methods in mitk::Surface.
*/
int mitkSurfaceEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkSurfaceEqualTest);

  Equal_InputIsNull_ReturnsFalse();
  Equal_CloneAndOriginalOneTimestep_ReturnsTrue();
  Equal_CloneAndOriginalTwoTimesteps_ReturnsTrue();
  Equal_OneTimeStepVSTwoTimeStep_ReturnsFalse();
  Equal_TwoTimeStepsDifferentPoints_ReturnsFalse();
  Equal_DifferentPoints_ReturnsFalse();
  Equal_SurfaceWithPolygonSurfaceWithPolyLine_ReturnsFalse();

  MITK_TEST_END();
}
