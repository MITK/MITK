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


#include "mitkCuboid.h"
#include "vtkLinearTransform.h"
#include "mitkVector.h"
#include "vtkCubeSource.h"

#include <vtkSTLReader.h>

mitk::Cuboid::Cuboid() 
: BoundingObject()
{
  vtkCubeSource* cube = vtkCubeSource::New();
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

bool mitk::Cuboid::IsInside(const Point3D& worldPoint) const
{
  // transform point from world to object coordinates
  Point3D p;
  GetGeometry(0)->WorldToIndex(worldPoint, p);

  bool retval =(p[0] >= -1) && (p[0] <= 1) 
            && (p[1] >= -1) && (p[1] <= 1) 
            && (p[2] >= -1) && (p[2] <= 1);
  return retval;
}

mitk::ScalarType mitk::Cuboid::GetVolume()
{
  Geometry3D* geometry = GetTimeSlicedGeometry();
  return   geometry->GetExtentInMM(0) 
         * geometry->GetExtentInMM(1)
         * geometry->GetExtentInMM(2);
}
