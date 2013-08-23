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
#include <vtkProperty.h>
#include <vtkProp.h>

mitk::VtkOverlay::VtkOverlay()
{
}


mitk::VtkOverlay::~VtkOverlay()
{
}

void mitk::VtkOverlay::Update(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkProp> prop = GetVtkProp(renderer);
  if(!IsVisible(renderer))
  {
    prop->SetVisibility(false);
    return;
  }
  else
  {

    prop->SetVisibility(true);
    UpdateVtkOverlay(renderer);
  }
}

void mitk::VtkOverlay::AddToBaseRenderer(mitk::BaseRenderer *renderer)
{
  Update(renderer);
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if(!renderer->GetVtkRenderer()->HasViewProp(vtkProp))
  {
    renderer->GetVtkRenderer()->AddViewProp(vtkProp);
  }
}

void mitk::VtkOverlay::RemoveFromBaseRenderer(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if(!renderer->GetVtkRenderer()->HasViewProp(vtkProp))
  {
    renderer->GetVtkRenderer()->RemoveViewProp(vtkProp);
  }
}
