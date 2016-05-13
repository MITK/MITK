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

#ifndef __QVTKFramebufferObjectRenderer_h
#define __QVTKFramebufferObjectRenderer_h

#include <QQuickFramebufferObject>
#include <vtkFrameBufferObject2.h>

#include "QVTKInternalOpenglRenderWindow.h"
#include "QVTKQuickItem.h"

#include <MitkOpenViewCoreExports.h>

class MITKOPENVIEWCORE_EXPORT QVTKFramebufferObjectRenderer : public QQuickFramebufferObject::Renderer
{
public:
  bool m_neverRendered;
  bool m_readyToRender;
  vtkInternalOpenGLRenderWindow *m_vtkRenderWindow;
  vtkFrameBufferObject2* m_vtkFBO;

  QVTKQuickItem *m_vtkQuickItem;

public:
  QVTKFramebufferObjectRenderer(vtkInternalOpenGLRenderWindow *rw);
  ~QVTKFramebufferObjectRenderer();

  virtual void synchronize(QQuickFramebufferObject * item);
  virtual void render();

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);

  friend class vtkInternalOpenGLRenderWindow;
};

#endif
