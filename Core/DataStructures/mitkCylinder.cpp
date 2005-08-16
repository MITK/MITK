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
{
  GetGeometry()->Initialize();
  vtkCylinderSource* cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.0);
  cylinder->SetHeight(2.0);
  cylinder->SetCenter(0.0, 0.0, 0.0);
  cylinder->SetResolution(20);
  cylinder->CappingOn();
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
	mitk::Point3D itkPoint;

  mitk::ScalarType v =  pow(p[0], 2) + pow(p[2], 2);
  bool retval = (v <= 1) && (p[1] >= -1) && (p[1] <= 1);
  return retval; 
}

mitk::ScalarType mitk::Cylinder::GetVolume()
{
  return   GetGeometry()->GetXAxis().GetNorm() * GetGeometry()->GetZAxis().GetNorm() 
         * 2 * GetGeometry()->GetYAxis().GetNorm() * vnl_math::pi;
}
