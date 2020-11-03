/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCuboid.h"
#include "mitkNumericTypes.h"
#include "vtkCubeSource.h"
#include "vtkLinearTransform.h"

#include <vtkSTLReader.h>

mitk::Cuboid::Cuboid() : BoundingObject()
{
  vtkCubeSource *cube = vtkCubeSource::New();
  cube->SetXLength(2.0);
  cube->SetYLength(2.0);
  cube->SetZLength(2.0);
  cube->Update();
  SetVtkPolyData(cube->GetOutput());
  cube->Delete();
}

mitk::Cuboid::~Cuboid()
{
}

bool mitk::Cuboid::IsInside(const Point3D &worldPoint) const
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;

  GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);

  return (p[0] >= -1) && (p[0] <= 1) && (p[1] >= -1) && (p[1] <= 1) && (p[2] >= -1) && (p[2] <= 1);
}

mitk::ScalarType mitk::Cuboid::GetVolume()
{
  TimeGeometry *geometry = GetTimeGeometry();
  return geometry->GetExtentInWorld(0) * geometry->GetExtentInWorld(1) * geometry->GetExtentInWorld(2);
}
