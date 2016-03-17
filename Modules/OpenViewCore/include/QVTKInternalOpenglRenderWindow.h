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

#ifndef __vtkInternalOpenGLRenderWindow_h
#define __vtkInternalOpenGLRenderWindow_h

#include <vtkGenericOpenGLRenderWindow.h>
#include <qopenglframebufferobject.h>
#include <QOpenGLFunctions>

class QVTKFramebufferObjectRenderer;

class vtkInternalOpenGLRenderWindow : public vtkGenericOpenGLRenderWindow
{
public:
  static vtkInternalOpenGLRenderWindow* New();
  vtkTypeMacro(vtkInternalOpenGLRenderWindow, vtkGenericOpenGLRenderWindow)

  virtual void OpenGLInitState();
  virtual void Render();
  void OpenGLEndState();
  void InternalRender();
  void SetFramebufferObject(QOpenGLFramebufferObject *fbo);

  QVTKFramebufferObjectRenderer *QtParentRenderer;

protected:
  vtkInternalOpenGLRenderWindow();
  ~vtkInternalOpenGLRenderWindow();
};

#endif
