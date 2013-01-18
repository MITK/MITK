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

#include "mitkVtkMapper.h"

mitk::VtkMapper::VtkMapper()
{
}

mitk::VtkMapper::~VtkMapper()
{
}

void mitk::VtkMapper::MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type){

 switch(type)
    {
    case mitk::VtkPropRenderer::Opaque: this->MitkRenderOpaqueGeometry(renderer); break;
    case mitk::VtkPropRenderer::Translucent: this->MitkRenderTranslucentGeometry(renderer); break;
    case mitk::VtkPropRenderer::Overlay:       this->MitkRenderOverlay(renderer); break;
    case mitk::VtkPropRenderer::Volumetric:    this->MitkRenderVolumetricGeometry(renderer); break;
    }

}


void mitk::VtkMapper::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}

void mitk::VtkMapper::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible( renderer )==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

void mitk::VtkMapper::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
  }
}

void mitk::VtkMapper::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
    return;
  if ( GetVtkProp(renderer)->GetVisibility() )
  {
    GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());
  }
}

bool mitk::VtkMapper::HasVtkProp( const vtkProp *prop, BaseRenderer *renderer )
{
  vtkProp *myProp = this->GetVtkProp( renderer );

  // TODO: check if myProp is a vtkAssembly and if so, check if prop is contained in its leafs
  return ( prop == myProp );
}

void mitk::VtkMapper::SetVtkMapperImmediateModeRendering(vtkMapper *mapper)
{
  if(mapper)
    mapper->SetImmediateModeRendering(mitk::VtkPropRenderer::useImmediateModeRendering());
}

void mitk::VtkMapper::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  vtkLinearTransform * vtktransform = GetDataNode()->GetVtkTransform(this->GetTimestep());

  vtkProp3D *prop = dynamic_cast<vtkProp3D*>( GetVtkProp(renderer) );
  if(prop)
    prop->SetUserTransform(vtktransform);
}

void mitk::VtkMapper::ApplyProperties(vtkActor* actor, BaseRenderer* renderer)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  this->GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  this->GetOpacity(rgba[3], renderer);

  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
}

void mitk::VtkMapper::ReleaseGraphicsResources(vtkWindow * /*renWin*/)
{
/*
  if(m_Prop3D)
    m_Prop3D->ReleaseGraphicsResources(renWin);
*/
}
