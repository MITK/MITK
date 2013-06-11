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
}


mitk::VtkOverlay3D::~VtkOverlay3D()
{
}


mitk::Overlay::Bounds mitk::VtkOverlay3D::GetBoundsOnDisplay(mitk::BaseRenderer*)
{
  mitk::Overlay::Bounds bounds;
  bounds.Position[0] = bounds.Position[1] = bounds.Size[0] = bounds.Size[1] = 0;
}

void mitk::VtkOverlay3D::SetBoundsOnDisplay(mitk::BaseRenderer*, mitk::Overlay::Bounds)
{
}

void mitk::VtkOverlay3D::SetPosition3D(mitk::Point3D position3D)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty);
}

void mitk::VtkOverlay3D::SetPosition3D(Point3D position3D, mitk::BaseRenderer *renderer)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty,renderer);
}

mitk::Point3D mitk::VtkOverlay3D::GetPosition3D()
{
  mitk::Point3D position3D;
  GetPropertyValue < mitk::Point3D > ("Pos3D", position3D);
  return position3D;
}

mitk::Point3D mitk::VtkOverlay3D::GetPosition3D(mitk::BaseRenderer *renderer)
{
  mitk::Point3D position3D;
  GetPropertyValue<mitk::Point3D>("Pos3D", position3D, renderer);
  return position3D;
}
