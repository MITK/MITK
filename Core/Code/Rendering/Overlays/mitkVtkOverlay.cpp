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

#include "mitkVtkOverlay.h"

mitk::VtkOverlay::VtkOverlay()
{
}


mitk::VtkOverlay::~VtkOverlay()
{
}

void mitk::VtkOverlay::UpdateOverlay(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkActor> vtkActor = GetVtkActor(renderer);
  if(!IsVisible(renderer))
  {
    vtkActor->SetVisibility(false);
    return;
  }
  else
  {
    vtkActor->SetVisibility(true);
    UpdateVtkOverlay(renderer);
  }
}

void mitk::VtkOverlay::AddOverlay(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if(!renderer->GetVtkRenderer()->HasViewProp(vtkProp))
  {
    renderer->GetVtkRenderer()->AddViewProp(vtkProp);
  }
}

void mitk::VtkOverlay::RemoveOverlay(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if(!renderer->GetVtkRenderer()->HasViewProp(vtkProp))
  {
    renderer->GetVtkRenderer()->RemoveViewProp(vtkProp);
  }
}
