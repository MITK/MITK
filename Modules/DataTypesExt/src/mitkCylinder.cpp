/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCylinder.h"
#include "mitkNumericTypes.h"
#include "vtkCylinderSource.h"
#include "vtkLinearTransform.h"

mitk::Cylinder::Cylinder() : BoundingObject()
{
  vtkCylinderSource *cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.0);
  cylinder->SetHeight(2.0);
  cylinder->SetCenter(0.0, 0.0, 0.0);
  cylinder->SetResolution(100);
  cylinder->CappingOn();
  cylinder->Update();
  SetVtkPolyData(cylinder->GetOutput());
  cylinder->Delete();
}

mitk::Cylinder::~Cylinder()
{
}

bool mitk::Cylinder::IsInside(const Point3D &worldPoint) const
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;

  GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);

  mitk::ScalarType v = pow(p[0], 2) + pow(p[2], 2);
  bool retval = (v <= 1) && (p[1] >= -1) && (p[1] <= 1);
  return retval;
}

mitk::ScalarType mitk::Cylinder::GetVolume()
{
  TimeGeometry *geometry = GetTimeGeometry();
  return geometry->GetExtentInWorld(0) * 0.5 * geometry->GetExtentInWorld(2) * 0.5 * vnl_math::pi *
         geometry->GetExtentInWorld(1);
}
