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

#include "mitkSurfaceIndexToWorldTransformFilter.h"

#include "vtkPoints.h"
#include "vtkPolyData.h"

mitk::SurfaceIndexToWorldTransformFilter::SurfaceIndexToWorldTransformFilter()
{
}

mitk::SurfaceIndexToWorldTransformFilter::~SurfaceIndexToWorldTransformFilter()
{
}

void mitk::SurfaceIndexToWorldTransformFilter::GenerateData()
{
  mitk::Surface* input = const_cast<mitk::Surface*>( this->GetInput());
  mitk::Surface::Pointer output = this->GetOutput();

  mitk::Point3D current, transformed;
  vtkPoints* surfacePoints = vtkPoints::New();
  vtkPoints* newSurfacePoints = vtkPoints::New();
  int pointIter = 0;

  surfacePoints = input->GetVtkPolyData()->GetPoints();

  while(pointIter != surfacePoints->GetNumberOfPoints())
  {
    double* intermed = surfacePoints->GetPoint(pointIter);
    current[0] = intermed[0];
    current[1] = intermed[1];
    current[2] = intermed[2];

    input->GetGeometry()->IndexToWorld(current, transformed);
    newSurfacePoints->InsertPoint( pointIter, transformed[0], transformed[1], transformed[2] );
    pointIter++;
  }

  output->GetVtkPolyData()->SetPoints(newSurfacePoints);
  output->GetVtkPolyData()->SetPolys(input->GetVtkPolyData()->GetPolys());
  output->GetGeometry()->SetIdentity();
  output->Modified();
  output->Update();
}


