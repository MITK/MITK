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


#include "mitkEllipsoid.h"
#include "vtkLinearTransform.h"
#include "mitkNumericTypes.h"
#include "vtkSphereSource.h"


mitk::Ellipsoid::Ellipsoid()
: BoundingObject()
{
  vtkSphereSource* sphere = vtkSphereSource::New();
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(20);
  sphere->SetPhiResolution(20);
  sphere->Update();
  SetVtkPolyData(sphere->GetOutput());
  sphere->Delete();
}



mitk::Ellipsoid::~Ellipsoid()
{
}


bool mitk::Ellipsoid::IsInside(const Point3D& worldPoint) const
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
  return   GetGeometry()->GetExtentInMM(0) * 0.5
    * GetGeometry()->GetExtentInMM(1) * 0.5
    * GetGeometry()->GetExtentInMM(2) * 0.5
    * vnl_math::pi * 4.0/3.0;
}
