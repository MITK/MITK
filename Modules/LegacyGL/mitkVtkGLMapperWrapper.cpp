/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkGLMapperWrapper.h"

// mitk includes
#include "mitkDataNode.h"
#include "mitkGL.h"
#include "vtkGLMapperProp.h"

// constructor LocalStorage
mitk::VtkGLMapperWrapper::LocalStorage::LocalStorage()
{
  m_GLMapperProp = vtkSmartPointer<vtkGLMapperProp>::New();
}
// destructor LocalStorage
mitk::VtkGLMapperWrapper::LocalStorage::~LocalStorage()
{
}

// constructor VtkGLMapperWrapper
mitk::VtkGLMapperWrapper::VtkGLMapperWrapper(GLMapper::Pointer mitkGLMapper)
{
  m_MitkGLMapper = mitkGLMapper;
}

// destructor
mitk::VtkGLMapperWrapper::~VtkGLMapperWrapper()
{
}

// returns propassembly
vtkProp *mitk::VtkGLMapperWrapper::GetVtkProp(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_GLMapperProp;
}

void mitk::VtkGLMapperWrapper::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_GLMapperProp->SetBaseRenderer(renderer);
  ls->m_GLMapperProp->SetWrappedGLMapper(m_MitkGLMapper);
}

void mitk::VtkGLMapperWrapper::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor)
{
  m_MitkGLMapper->ApplyColorAndOpacityProperties(renderer, actor);
}

void mitk::VtkGLMapperWrapper::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type)
{
  if (type != mitk::VtkPropRenderer::Opaque)
    return;
  Enable2DOpenGL(renderer);
  Superclass::MitkRender(renderer, type);
  Disable2DOpenGL();
}

void mitk::VtkGLMapperWrapper::Update(mitk::BaseRenderer *renderer)
{
  Superclass::Update(renderer);
  m_MitkGLMapper->Update(renderer);
}

void mitk::VtkGLMapperWrapper::SetDataNode(mitk::DataNode *node)
{
  m_MitkGLMapper->SetDataNode(node);
}

mitk::DataNode *mitk::VtkGLMapperWrapper::GetDataNode() const
{
  return m_MitkGLMapper->GetDataNode();
}

/*!
\brief
Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D
rendering (perspective projection)
*/
void mitk::VtkGLMapperWrapper::Enable2DOpenGL(mitk::BaseRenderer *)
{
  GLint vp[4];

  // Get a copy of the viewport
  glGetIntegerv(GL_VIEWPORT, vp);

  // Save a copy of the projection matrix so that we can restore it
  // when it's time to do 3D rendering again.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho(vp[0], vp[2] + vp[0], vp[1], vp[3] + vp[1], -2000, 2000);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Make sure depth testing and lighting are disabled for 2D rendering until
  // we are finished rendering in 2D
  glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  // disable the texturing here so crosshair is painted in the correct colors
  // vtk will reenable texturing every time it is needed
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glLineWidth(1.0);
}

/*!
\brief Initialize the VtkPropRenderer

Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D
rendering (perspective projection)
*/
void mitk::VtkGLMapperWrapper::Disable2DOpenGL()
{
  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
