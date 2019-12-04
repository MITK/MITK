/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEllipsoid.h"
#include "mitkNumericTypes.h"
#include "vtkLinearTransform.h"
#include <vtkSphereSource.h>

mitk::Ellipsoid::Ellipsoid() : BoundingObject()
{
  auto sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(20);
  sphere->SetPhiResolution(20);
  sphere->Update();
  SetVtkPolyData(sphere->GetOutput());
}

mitk::Ellipsoid::~Ellipsoid()
{
}

bool mitk::Ellipsoid::IsInside(const Point3D &worldPoint) const
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;

  GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);

  return (pow(p[0], 2) + pow(p[1], 2) + pow(p[2], 2) <= 1);
}

mitk::ScalarType mitk::Ellipsoid::GetVolume()
{
  return GetGeometry()->GetExtentInMM(0) * 0.5 * GetGeometry()->GetExtentInMM(1) * 0.5 *
         GetGeometry()->GetExtentInMM(2) * 0.5 * vnl_math::pi * 4.0 / 3.0;
}
