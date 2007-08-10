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


#include "mitkCylinder.h"
#include "vtkLinearTransform.h"
#include "mitkVector.h"
#include "vtkCylinderSource.h"


mitk::Cylinder::Cylinder()
: BoundingObject()
{
  vtkCylinderSource* cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.0);
  cylinder->SetHeight(2.0);
  cylinder->SetCenter(0.0, 0.0, 0.0);
  cylinder->SetResolution(20);
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
  Point3D p;
  GetGeometry(0)->WorldToIndex(worldPoint, p);

  mitk::ScalarType v =  pow(p[0], 2) + pow(p[2], 2);
  bool retval = (v <= 1) && (p[1] >= -1) && (p[1] <= 1);
  return retval; 
}


mitk::ScalarType mitk::Cylinder::GetVolume()
{
  Geometry3D* geometry = GetTimeSlicedGeometry();
  return   geometry->GetExtentInMM(0) * 0.5
         * geometry->GetExtentInMM(2) * 0.5
         * vnl_math::pi
         * geometry->GetExtentInMM(1);
}
