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


#include "mitkCylinder.h"
#include "vtkLinearTransform.h"
#include "mitkNumericTypes.h"
#include "vtkCylinderSource.h"


mitk::Cylinder::Cylinder()
: BoundingObject()
{
  vtkCylinderSource* cylinder = vtkCylinderSource::New();
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


bool mitk::Cylinder::IsInside(const Point3D& worldPoint) const
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;

  GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);

  mitk::ScalarType v =  pow(p[0], 2) + pow(p[2], 2);
  bool retval = (v <= 1) && (p[1] >= -1) && (p[1] <= 1);
  return retval;
}


mitk::ScalarType mitk::Cylinder::GetVolume()
{
  TimeGeometry* geometry = GetTimeGeometry();
  return   geometry->GetExtentInWorld(0) * 0.5
    * geometry->GetExtentInWorld(2) * 0.5
    * vnl_math::pi
    * geometry->GetExtentInWorld(1);
}
