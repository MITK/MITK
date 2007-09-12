/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <vtkProp3D.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>

//##ModelId=3E3424950213
mitk::BaseVtkMapper3D::BaseVtkMapper3D() : m_SliceNr(0), m_TimeNr(0), m_ChannelNr(0), m_Prop3D(NULL)
{
  //vtkMapper::GlobalImmediateModeRenderingOn();
}

//##ModelId=3E3424950231
mitk::BaseVtkMapper3D::~BaseVtkMapper3D()
{
  if(m_Prop3D)
    m_Prop3D->Delete();
}

vtkProp* mitk::BaseVtkMapper3D::GetProp()
{
  if(GetDataTreeNode()!=NULL && 
    m_Prop3D != NULL) {
      m_Prop3D->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
  } 
  return m_Prop3D;
}

void mitk::BaseVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer* renderer)
{
  int timeStep = renderer->GetTimeStep(GetDataTreeNode()->GetData());
  vtkLinearTransform * vtktransform = GetDataTreeNode()->GetVtkTransform(timeStep);
  m_Prop3D->SetUserTransform(vtktransform);
}

//##ModelId=3EF180C70065
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
