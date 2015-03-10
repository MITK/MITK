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
#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkTestingMacros.h"

#include "vtkPolyData.h"
#include "vtkSphereSource.h"

#include <fstream>

int mitkSurfaceTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("Surface");

  mitk::Surface::Pointer surface = mitk::Surface::New();
  MITK_TEST_CONDITION_REQUIRED( surface.GetPointer(), "Testing initialization!" );

  mitk::Surface::Pointer cloneSurface = surface->Clone();
  MITK_TEST_CONDITION_REQUIRED( cloneSurface.GetPointer(), "Testing clone surface initialization!" );

  vtkSphereSource* sphereSource = vtkSphereSource::New();
  sphereSource->SetCenter(0,0,0);
  sphereSource->SetRadius(5.0);
  sphereSource->SetThetaResolution(10);
  sphereSource->SetPhiResolution(10);
  sphereSource->Update();

  vtkPolyData* polys = sphereSource->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(surface->GetVtkPolyData() == NULL, "Testing initial state of vtkPolyData");
  surface->SetVtkPolyData( polys );
  sphereSource->Delete();
  MITK_TEST_CONDITION_REQUIRED(surface->GetVtkPolyData()!= NULL, "Testing set vtkPolyData");

  cloneSurface= NULL;
  cloneSurface = surface->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneSurface->GetVtkPolyData()!= NULL, "Testing set vtkPolyData of cloned surface!");
  cloneSurface = NULL;

    double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    polys->ComputeBounds();
    polys->GetBounds( bounds );

    surface->UpdateOutputInformation();
    surface->SetRequestedRegionToLargestPossibleRegion();
    mitk::BoundingBox* bb = const_cast<mitk::BoundingBox*>(surface->GetGeometry()->GetBoundingBox());
    mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

    bool passed = false;
    if ( bounds[0] == surfBounds[0] && bounds[1] == surfBounds[1]
      && bounds[2] == surfBounds[2] && bounds[3] == surfBounds[3]
      && bounds[4] == surfBounds[4] && bounds[5] == surfBounds[5] )
    {
      passed = true;
    }
    MITK_TEST_CONDITION_REQUIRED(passed, "Testing GetBoundingBox()!");

  surface->Expand(5);
  surface->Update();
  surface->SetRequestedRegionToLargestPossibleRegion();
  mitk::Surface::RegionType requestedRegion = surface->GetRequestedRegion();
  MITK_TEST_CONDITION_REQUIRED(requestedRegion.GetSize(3) == 5, "Testing mitk::Surface::Expand( timesteps ): ");

  double boundsMat[5][6];

  for (int i=0;i<5;i++) {
    vtkSphereSource* sphereSource = vtkSphereSource::New();
    sphereSource->SetCenter(0,0,0);
    sphereSource->SetRadius(1.0 * (i+1.0));
    sphereSource->SetThetaResolution(10);
    sphereSource->SetPhiResolution(10);
    sphereSource->Update();
    sphereSource->GetOutput()->ComputeBounds();
    sphereSource->GetOutput()->GetBounds( boundsMat[i] );
    surface->SetVtkPolyData( sphereSource->GetOutput(),i );
    sphereSource->Delete();
  }

  surface->UpdateOutputInformation();
  surface->SetRequestedRegionToLargestPossibleRegion();

  passed = true;
  for (int i=0;i<5;i++)
  {
    mitk::BoundingBox::BoundsArrayType surfBounds = (const_cast<mitk::BoundingBox*>(surface->GetTimeGeometry()->GetGeometryForTimeStep(i)->GetBoundingBox()))->GetBounds();

    if ( boundsMat[i][0] != surfBounds[0]
    || boundsMat[i][1] != surfBounds[1]
    || boundsMat[i][2] != surfBounds[2]
    || boundsMat[i][3] != surfBounds[3]
    || boundsMat[i][4] != surfBounds[4]
    || boundsMat[i][5] != surfBounds[5] )
    {
      passed = false;
      break;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(passed, "Testing mitk::Surface::Testing 4D surface data creation!" );

  const mitk::TimeGeometry* inputTimeGeometry = surface->GetUpdatedTimeGeometry();

  int time = 3;
  int timestep=0;
  timestep = inputTimeGeometry->TimePointToTimeStep( time );
  MITK_TEST_CONDITION_REQUIRED(time == timestep, "Testing correctness of geometry for surface->GetUpdatedTimeGeometry()!");

  sphereSource = vtkSphereSource::New();
  sphereSource->SetCenter(0,0,0);
  sphereSource->SetRadius( 100.0 );
  sphereSource->SetThetaResolution(10);
  sphereSource->SetPhiResolution(10);
  sphereSource->Update();
  surface->SetVtkPolyData( sphereSource->GetOutput(), 3 );
  sphereSource->Delete();

  inputTimeGeometry = surface->GetUpdatedTimeGeometry();
  time = 3;
  timestep=0;

  timestep = inputTimeGeometry->TimePointToTimeStep( time );
  MITK_TEST_CONDITION_REQUIRED(time == timestep, "Explicitly changing the data of timestep 3 and checking for timebounds correctness of surface's geometry again!");

  unsigned int numberoftimesteps = surface->GetTimeSteps();
  mitk::Surface::Pointer dummy = mitk::Surface::New();
  dummy->Graft(surface);
  MITK_TEST_CONDITION_REQUIRED( dummy->GetVtkPolyData() != NULL, "Testing copying a Surface with Graft()!");
  MITK_TEST_CONDITION_REQUIRED( dummy->GetTimeSteps() == numberoftimesteps, "orig-numberofTimeSteps:" << numberoftimesteps << "  copy-numberofTimeSteps:" << dummy->GetTimeSteps());

  surface = NULL;
  MITK_TEST_CONDITION_REQUIRED( surface.IsNull(), "Testing destruction of surface!");

  MITK_TEST_END();
}
