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

#include "mitkVtkAnnotation2D.h"
#include <vtkActor2D.h>
#include <vtkCoordinate.h>
#include <vtkProperty2D.h>

mitk::VtkAnnotation2D::VtkAnnotation2D()
{
}

mitk::VtkAnnotation2D::~VtkAnnotation2D()
{
}

mitk::Annotation::Bounds mitk::VtkAnnotation2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  mitk::Annotation::Bounds bounds;
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  bounds.Position = actor->GetPosition();
  bounds.Size = actor->GetPosition2();
  return bounds;
}

void mitk::VtkAnnotation2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Annotation::Bounds &bounds)
{
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  actor->SetDisplayPosition(bounds.Position[0], bounds.Position[1]);
  actor->SetWidth(bounds.Size[0]);
  actor->SetHeight(bounds.Size[1]);
}

void mitk::VtkAnnotation2D::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  vtkActor2D *prop = GetVtkActor2D(renderer);
  float color[3] = {1, 1, 1};
  float opacity = 1.0;
  GetColor(color);
  GetOpacity(opacity);
  prop->GetProperty()->SetColor(color[0], color[1], color[2]);
  prop->GetProperty()->SetOpacity(opacity);
  UpdateVtkAnnotation2D(renderer);
}

void mitk::VtkAnnotation2D::SetPosition2D(const Point2D &position2D)
{
  mitk::Point2dProperty::Pointer position2dProperty = mitk::Point2dProperty::New(position2D);
  SetProperty("VtkAnnotation2D.Position2D", position2dProperty.GetPointer());
}

mitk::Point2D mitk::VtkAnnotation2D::GetPosition2D() const
{
  mitk::Point2D position2D;
  position2D.Fill(0);
  GetPropertyValue<mitk::Point2D>("VtkAnnotation2D.Position2D", position2D);
  return position2D;
}

void mitk::VtkAnnotation2D::SetOffsetVector(const Point2D &OffsetVector)
{
  mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
  SetProperty("VtkAnnotation2D.OffsetVector", OffsetVectorProperty.GetPointer());
}

mitk::Point2D mitk::VtkAnnotation2D::GetOffsetVector() const
{
  mitk::Point2D OffsetVector;
  OffsetVector.Fill(0);
  GetPropertyValue<mitk::Point2D>("VtkAnnotation2D.OffsetVector", OffsetVector);
  return OffsetVector;
}

vtkProp *mitk::VtkAnnotation2D::GetVtkProp(mitk::BaseRenderer *renderer) const
{
  return GetVtkActor2D(renderer);
}
