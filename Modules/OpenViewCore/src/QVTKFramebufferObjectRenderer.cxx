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

#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkMutexLock.h>

#include <vtkStructuredPointsReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageClip.h>
#include <vtkVolumeProperty.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolume.h>

QVTKFramebufferObjectRenderer::QVTKFramebufferObjectRenderer(vtkInternalOpenGLRenderWindow *rw) :
m_vtkRenderWindow(rw),
m_neverRendered(true),
m_readyToRender(false)
{
  m_vtkRenderWindow->Register(NULL);
  m_vtkRenderWindow->QtParentRenderer = this;
}

QOpenGLFramebufferObject * QVTKFramebufferObjectRenderer::createFramebufferObject(const QSize &size)
{

  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::Depth);
  format.setTextureTarget(GL_TEXTURE_2D);
  format.setInternalTextureFormat(GL_RGBA32F_ARB);
  QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(size, format);

  m_vtkRenderWindow->SetFramebufferObject(fbo);
  return fbo;
}

void QVTKFramebufferObjectRenderer::render()
{
  if (!m_readyToRender)
  {
    return;
  }

  m_vtkQuickItem->m_viewLock.lock();
  m_vtkRenderWindow->PushState();
  m_vtkRenderWindow->OpenGLInitState();
  m_vtkRenderWindow->InternalRender();
  m_vtkRenderWindow->OpenGLEndState();
  m_vtkRenderWindow->PopState();
  m_vtkQuickItem->m_viewLock.unlock();

}

void QVTKFramebufferObjectRenderer::synchronize(QQuickFramebufferObject * item)
{
  m_vtkQuickItem = static_cast<QVTKQuickItem*>(item);

  if (m_neverRendered)
  {
    m_neverRendered = false;
    m_vtkQuickItem->init();
  }
  m_readyToRender = m_vtkQuickItem->prepareForRender();
}

QVTKFramebufferObjectRenderer::~QVTKFramebufferObjectRenderer()
{
  m_vtkRenderWindow->QtParentRenderer = 0;
  m_vtkRenderWindow->Delete();
}
