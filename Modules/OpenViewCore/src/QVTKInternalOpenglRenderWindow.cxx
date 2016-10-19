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

#include "QVTKInternalOpenglRenderWindow.h"
#include "QVTKFramebufferObjectRenderer.h"

#include <vtkgl.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkInternalOpenGLRenderWindow);

vtkInternalOpenGLRenderWindow::~vtkInternalOpenGLRenderWindow()
{
  this->OffScreenRendering = false;
}

vtkInternalOpenGLRenderWindow::vtkInternalOpenGLRenderWindow() :
QtParentRenderer(0)
{
}

void vtkInternalOpenGLRenderWindow::InternalRender()
{
  Superclass::Render();
}

void vtkInternalOpenGLRenderWindow::OpenGLEndState()
{
  glDepthMask(GL_TRUE);
}

void vtkInternalOpenGLRenderWindow::OpenGLInitState()
{
  Superclass::OpenGLInitState();
  vtkgl::UseProgram(0);

  glEnable(GL_BLEND);
  glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
  glDepthMask(GL_TRUE);
}

void vtkInternalOpenGLRenderWindow::Render()
{
  if (this->QtParentRenderer)
  {
    this->QtParentRenderer->update();
  }
}

void vtkInternalOpenGLRenderWindow::SetFramebufferObject(QOpenGLFramebufferObject *fbo)
{
  this->SetFrontBuffer(vtkgl::COLOR_ATTACHMENT0);
  this->SetFrontRightBuffer(vtkgl::COLOR_ATTACHMENT0);
  this->SetBackLeftBuffer(vtkgl::COLOR_ATTACHMENT0);
  this->SetBackRightBuffer(vtkgl::COLOR_ATTACHMENT0);

  QSize fboSize = fbo->size();
  this->SetSize(fboSize.width(), fboSize.height());

  this->NumberOfFrameBuffers    = 1;
  this->FrameBufferObject       = static_cast<unsigned int>(fbo->handle());
  this->DepthRenderBufferObject = 0; // static_cast<unsigned int>(depthRenderBufferObject);
  this->TextureObjects[0]       = static_cast<unsigned int>(fbo->texture());
  this->OffScreenRendering	  = true;
  this->OffScreenUseFrameBuffer = true;

  this->Modified();
}
