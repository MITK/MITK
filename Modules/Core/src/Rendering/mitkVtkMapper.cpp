/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVtkMapper.h>

mitk::VtkMapper::VtkMapper()
{
}

mitk::VtkMapper::~VtkMapper()
{
}

int mitk::VtkMapper::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type)
{
  switch (type)
  {
    case mitk::VtkPropRenderer::Opaque:
      return this->MitkRenderOpaqueGeometry(renderer);
    case mitk::VtkPropRenderer::Translucent:
      return this->MitkRenderTranslucentGeometry(renderer);
    case mitk::VtkPropRenderer::Overlay:
      return this->MitkRenderOverlay(renderer);
    case mitk::VtkPropRenderer::Volumetric:
      return this->MitkRenderVolumetricGeometry(renderer);
  }

  return 0;
}

int mitk::VtkMapper::MitkRenderOverlay(BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return 0;

  if (this->GetVtkProp(renderer)->GetVisibility())
    return this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());

  return 0;
}

int mitk::VtkMapper::MitkRenderOpaqueGeometry(BaseRenderer *renderer)
{
  bool visible = true;

  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return 0;

  if (this->GetVtkProp(renderer)->GetVisibility())
    return this->GetVtkProp(renderer)->RenderOpaqueGeometry(renderer->GetVtkRenderer());

  return 0;
}

int mitk::VtkMapper::MitkRenderTranslucentGeometry(BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return 0;

  if (this->GetVtkProp(renderer)->GetVisibility())
    return this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());

  return 0;
}

int mitk::VtkMapper::MitkRenderVolumetricGeometry(BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return 0;

  if (this->GetVtkProp(renderer)->GetVisibility())
    return this->GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());

  return 0;
}

bool mitk::VtkMapper::HasVtkProp(const vtkProp *prop, BaseRenderer *renderer)
{
  vtkProp *myProp = this->GetVtkProp(renderer);

  // TODO: check if myProp is a vtkAssembly and if so, check if prop is contained in its leafs
  return (prop == myProp);
}

void mitk::VtkMapper::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  vtkLinearTransform *vtktransform = GetDataNode()->GetVtkTransform(this->GetTimestep());

  auto *prop = dynamic_cast<vtkProp3D *>(GetVtkProp(renderer));
  if (prop)
    prop->SetUserTransform(vtktransform);
}

void mitk::VtkMapper::ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor)
{
  float rgba[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  DataNode *node = GetDataNode();

  // check for color prop and use it for rendering if it exists
  node->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  node->GetOpacity(rgba[3], renderer, "opacity");

  double drgba[4] = {rgba[0], rgba[1], rgba[2], rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
}

void mitk::VtkMapper::SetOpacityAndRenderPass(vtkActor *actor, double opacity)
{
  actor->GetProperty()->SetOpacity(opacity);
  actor->SetForceOpaque(opacity >= 1.0);
  actor->SetForceTranslucent(opacity < 1.0);
}
