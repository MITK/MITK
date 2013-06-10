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


vtkSmartPointer<vtkProp> mitk::VtkOverlay2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return GetVtkActor2D(renderer);
}
