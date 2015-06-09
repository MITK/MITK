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

#include "vtkGLMapperProp.h"

#include <vtkObjectFactory.h>


vtkStandardNewMacro(vtkGLMapperProp);

vtkGLMapperProp::vtkGLMapperProp() :
  m_WrappedGLMapper(0),
  m_BaseRenderer(0)
{
}

vtkGLMapperProp::~vtkGLMapperProp()
{
}

int vtkGLMapperProp::RenderOpaqueGeometry(vtkViewport *)
{
  if(!m_WrappedGLMapper || !m_BaseRenderer)
    return 0;

  this->m_WrappedGLMapper->MitkRender(m_BaseRenderer, mitk::VtkPropRenderer::Opaque);
  return 1;
}

int vtkGLMapperProp::RenderTranslucentPolygonalGeometry(vtkViewport *)
{
  return 0;
}

int vtkGLMapperProp::RenderVolumetricGeometry(vtkViewport *)
{
  return 0;
}

int vtkGLMapperProp::RenderOverlay(vtkViewport *)
{
  return 0;
}

const mitk::GLMapper *vtkGLMapperProp::GetWrappedGLMapper() const
{
  return m_WrappedGLMapper;
}

void vtkGLMapperProp::SetWrappedGLMapper(mitk::GLMapper* glMapper)
{
  this->m_WrappedGLMapper = glMapper;
}

void vtkGLMapperProp::SetBaseRenderer(mitk::BaseRenderer *baseRenderer)
{
  this->m_BaseRenderer = baseRenderer;
}
