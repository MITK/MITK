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

#include "mitkVtkAnnotation.h"
#include <vtkProp.h>
#include <vtkProperty.h>

mitk::VtkAnnotation::VtkAnnotation()
{
}

mitk::VtkAnnotation::~VtkAnnotation()
{
}

void mitk::VtkAnnotation::Update(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkProp> prop = GetVtkProp(renderer);
  if (!IsVisible())
  {
    prop->SetVisibility(false);
    return;
  }
  else
  {
    prop->SetVisibility(true);
    UpdateVtkAnnotation(renderer);
  }
}

void mitk::VtkAnnotation::AddToBaseRenderer(mitk::BaseRenderer *renderer)
{
  if (!renderer)
    return;
  AddToRenderer(renderer, renderer->GetVtkRenderer());
}

void mitk::VtkAnnotation::AddToRenderer(mitk::BaseRenderer *renderer, vtkRenderer *vtkrenderer)
{
  if (!renderer || !vtkrenderer)
    return;
  Update(renderer);
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if (renderer && vtkrenderer && !vtkrenderer->HasViewProp(vtkProp))
  {
    vtkrenderer->AddViewProp(vtkProp);
    mitk::RenderingManager::GetInstance()->RequestUpdate(vtkrenderer->GetRenderWindow());
  }
}

void mitk::VtkAnnotation::RemoveFromBaseRenderer(mitk::BaseRenderer *renderer)
{
  if (!renderer)
    return;
  RemoveFromRenderer(renderer, renderer->GetVtkRenderer());
}

void mitk::VtkAnnotation::RemoveFromRenderer(mitk::BaseRenderer *renderer, vtkRenderer *vtkrenderer)
{
  if (!renderer || !vtkrenderer)
    return;
  vtkSmartPointer<vtkProp> vtkProp = GetVtkProp(renderer);
  if (vtkrenderer->HasViewProp(vtkProp))
  {
    vtkrenderer->RemoveViewProp(vtkProp);
    mitk::RenderingManager::GetInstance()->RequestUpdate(vtkrenderer->GetRenderWindow());
  }
}

void mitk::VtkAnnotation::Paint(BaseRenderer *renderer)
{
  GetVtkProp(renderer)->RenderOpaqueGeometry(renderer->GetVtkRenderer());
  GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
}
