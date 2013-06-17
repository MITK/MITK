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

#include "mitkVtkOverlay2D.h"
#include <vtkCoordinate.h>

mitk::VtkOverlay2D::VtkOverlay2D()
{
}


mitk::VtkOverlay2D::~VtkOverlay2D()
{
}


mitk::Overlay::Bounds mitk::VtkOverlay2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer)
{
  mitk::Overlay::Bounds bounds;
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  bounds.Position = actor->GetPosition();
  bounds.Size = actor->GetPosition2();
}

void mitk::VtkOverlay2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, mitk::Overlay::Bounds bounds)
{
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  actor->SetPosition(bounds.Position[0],bounds.Position[1]);
  actor->SetPosition2(bounds.Size[0],bounds.Size[1]);
}

void mitk::VtkOverlay2D::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkActor2D> prop = GetVtkActor2D(renderer);
  float color[3] = {1,1,1};
  float opacity = 1.0;
  GetColor(color,renderer);
  GetOpacity(opacity,renderer);
  prop->GetProperty()->SetColor(color[0], color[1], color[2]);
  prop->GetProperty()->SetOpacity(opacity);
  UpdateVtkOverlay2D(renderer);
}


void mitk::VtkOverlay2D::SetPosition2D(Point2D position2D, mitk::BaseRenderer *renderer)
{
  mitk::Point2dProperty::Pointer position2dProperty = mitk::Point2dProperty::New(position2D);
  SetProperty("Position2D", position2dProperty,renderer);
}

mitk::Point2D mitk::VtkOverlay2D::GetPosition2D(mitk::BaseRenderer *renderer)
{
  mitk::Point2D position2D;
  GetPropertyValue<mitk::Point2D>("Position2D", position2D, renderer);
  return position2D;
}

void mitk::VtkOverlay2D::SetOffsetVector(Point2D OffsetVector, mitk::BaseRenderer *renderer)
{
  mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
  SetProperty("OffsetVector", OffsetVectorProperty,renderer);
}

mitk::Point2D mitk::VtkOverlay2D::GetOffsetVector(mitk::BaseRenderer *renderer)
{
  mitk::Point2D OffsetVector;
  GetPropertyValue<mitk::Point2D>("OffsetVector", OffsetVector, renderer);
  return OffsetVector;
}


vtkSmartPointer<vtkProp> mitk::VtkOverlay2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return GetVtkActor2D(renderer);
}
