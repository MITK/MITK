/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBaseVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include <vtkProp3D.h>
#include <vtkLODProp3D.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>

mitk::BaseVtkMapper3D::BaseVtkMapper3D() : m_Prop3D(NULL)
{
  //vtkMapper::GlobalImmediateModeRenderingOn();
}

mitk::BaseVtkMapper3D::~BaseVtkMapper3D()
{
  if(m_Prop3D)
    m_Prop3D->Delete();
}

vtkProp* mitk::BaseVtkMapper3D::GetProp()
{
  return m_Prop3D;
}

void mitk::BaseVtkMapper3D::UpdateVtkTransform()
{
  vtkLinearTransform * vtktransform = GetDataTreeNode()->GetVtkTransform(this->GetTimestep());
  m_Prop3D->SetUserTransform(vtktransform);
}

void mitk::BaseVtkMapper3D::MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false) 
    return;
  
  if(GetProp()->GetVisibility())
    GetProp()->RenderOpaqueGeometry(renderer->GetVtkRenderer());
}

void mitk::BaseVtkMapper3D::MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false) 
    return;
  
 /* if(dynamic_cast<vtkLODProp3D*>(m_Prop3D) != NULL)
  {
    if(  dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())==NULL ||  
         dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false)    
       return;
  }*/
  
   if(GetProp()->GetVisibility())
    GetProp()->RenderTranslucentGeometry(renderer->GetVtkRenderer());
}

void mitk::BaseVtkMapper3D::MitkRenderOverlay(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false) 
    return;
  
  if(GetProp()->GetVisibility())
    GetProp()->RenderOverlay(renderer->GetVtkRenderer());
}

void mitk::BaseVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  GetOpacity(rgba[3], renderer);

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
#else
  actor->GetProperty()->SetColor(rgba);
  actor->GetProperty()->SetOpacity(rgba[3]);
#endif
}
