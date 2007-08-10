/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkCone.h"
#include "vtkLinearTransform.h"
#include "mitkVector.h"
#include "vtkConeSource.h"

mitk::Cone::Cone()
: BoundingObject()
{
  // Set up Cone Surface. Radius 1.0, height 2.0, , centered around the origin
  vtkConeSource* cone = vtkConeSource::New();
  cone->SetRadius(1.0);
  cone->SetHeight(2.0);
  cone->SetDirection(0.0, -1.0, 0.0);
  cone->SetCenter(0.0, 0.0, 0.0);
  cone->SetResolution(20);
  cone->CappingOn();
  cone->Update();
  SetVtkPolyData(cone->GetOutput());
  cone->Delete(); 
}

mitk::Cone::~Cone()
{
}

bool mitk::Cone::IsInside(const Point3D& worldPoint) const
{
  // transform point from world to object coordinates
  Point3D p;
  GetGeometry(0)->WorldToIndex(worldPoint, p);

  p[1] += 1;  // translate point, so that it fits to the formula below, which describes a cone that has its cone vertex at the origin                                            
  return (sqrt(p[0] * p[0] + p[2] * p[2]) <= p[1] * 0.5) && (p[1] <= 2);  // formula to calculate if a given point is inside a cone that has its cone vertex at the origin, is aligned on the second axis, has a radius of one an a height of two
}

mitk::ScalarType mitk::Cone::GetVolume()
{
  Geometry3D* geometry = GetTimeSlicedGeometry();
  return   geometry->GetExtentInMM(0) * 0.5
         * geometry->GetExtentInMM(2) * 0.5
         * vnl_math::pi / 3.0
         * geometry->GetExtentInMM(1);
}
