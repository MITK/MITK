/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-08-17 16:41:18 +0200 (Fr, 17 Aug 2007) $
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMitkRenderProp.h"
#include "vtkObjectFactory.h"
#include "vtkLODProp3D.h"

vtkStandardNewMacro(vtkMitkRenderProp);

vtkMitkRenderProp::vtkMitkRenderProp()
{
}
vtkMitkRenderProp::~vtkMitkRenderProp()
{
}

double *vtkMitkRenderProp::GetBounds()
{
  return const_cast<double*>(m_VtkPropRenderer->GetBounds());
}

void vtkMitkRenderProp::SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer)
{
  this->m_VtkPropRenderer = propRenderer;
}

int vtkMitkRenderProp::RenderOpaqueGeometry(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Opaque); 
}
int vtkMitkRenderProp::RenderTranslucentGeometry(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Translucent);
}
int vtkMitkRenderProp::RenderOverlay(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Overlay);
}

void vtkMitkRenderProp::ReleaseGraphicsResources(vtkWindow* /*window*/)
{ //B/ what shall we do here....
}
