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

#include "mitkVtkOverlay3D.h"

mitk::VtkOverlay3D::VtkOverlay3D()
{
  mitk::Point3D offsetVector;
  offsetVector.Fill(0);
  SetOffsetVector(offsetVector);
}


mitk::VtkOverlay3D::~VtkOverlay3D()
{
}

void mitk::VtkOverlay3D::SetPosition3D(const Point3D& position3D, mitk::BaseRenderer *renderer)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("VtkOverlay3D.Position3D", position3dProperty.GetPointer(),renderer);
}

mitk::Point3D mitk::VtkOverlay3D::GetPosition3D(mitk::BaseRenderer *renderer) const
{
  mitk::Point3D position3D;
  position3D.Fill(0);
  GetPropertyValue<mitk::Point3D>("VtkOverlay3D.Position3D", position3D, renderer);
  return position3D;
}

void mitk::VtkOverlay3D::SetOffsetVector(const Point3D& OffsetVector, mitk::BaseRenderer *renderer)
{
  mitk::Point3dProperty::Pointer OffsetVectorProperty = mitk::Point3dProperty::New(OffsetVector);
  SetProperty("VtkOverlay3D.OffsetVector", OffsetVectorProperty.GetPointer(),renderer);
}

mitk::Point3D mitk::VtkOverlay3D::GetOffsetVector(mitk::BaseRenderer *renderer) const
{
  mitk::Point3D OffsetVector;
  OffsetVector.Fill(0);
  GetPropertyValue<mitk::Point3D>("VtkOverlay3D.OffsetVector", OffsetVector, renderer);
  return OffsetVector;
}
