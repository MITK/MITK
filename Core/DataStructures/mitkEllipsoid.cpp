/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkEllipsoid.h"
#include "vtkTransform.h"
#include "mitkVector.h"
#include "vtkSphereSource.h"

mitk::Ellipsoid::Ellipsoid():BoundingObject()
{
  //m_Geometry3D->Initialize();
  vtkSphereSource* sphere = vtkSphereSource::New();
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(20);
  sphere->SetPhiResolution(20);
  sphere->Update();
  SetVtkPolyData(sphere->GetOutput());
  std::cout << GetVtkPolyData()<< ", " << sphere->GetOutput() << std::endl;
  //sphere->Delete();
}

mitk::Ellipsoid::~Ellipsoid()
{
}

bool mitk::Ellipsoid::IsInside(Point3D worldPoint)
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;
  m_Geometry3D->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
	mitk::Point3D itkPoint;
  return (pow(p[0], 2) + pow(p[1], 2) + pow(p[2], 2) <= 1);
}


mitk::ScalarType mitk::Ellipsoid::GetVolume()
{
  return   m_Geometry3D->GetXAxis().GetNorm() * m_Geometry3D->GetYAxis().GetNorm() 
         * m_Geometry3D->GetZAxis().GetNorm() * vnl_math::pi * 4.0/3.0;
}
