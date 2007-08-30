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


#ifndef QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66
#define QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66

#include <qgl.h>
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"

/**
 * \brief Qt implementation of mitk::RenderWindow
 * \ingroup Renderer
 */
class QmitkRenderWindow : public QGLWidget, public mitk::RenderWindow
{
public:
  QmitkRenderWindow(mitk::BaseRenderer* renderer, QGLFormat glf, QWidget *parent = 0, const char *name = 0);

  QmitkRenderWindow(QGLFormat glf, QWidget *parent = 0, const char *name = 0);

  QmitkRenderWindow(mitk::BaseRenderer* renderer, QWidget *parent = 0, const char *name = 0);

  QmitkRenderWindow(QWidget *parent = 0, const char *name = 0);

  virtual ~QmitkRenderWindow();

  virtual void MakeCurrent();

  virtual void SwapBuffers ();

  virtual bool IsSharing () const;

  virtual QSize minimumSizeHint () const;

  virtual QSizePolicy sizePolicy() const;

  virtual QSize sizeHint () const;


  virtual void mousePressEvent(QMouseEvent*);

  virtual void mouseReleaseEvent(QMouseEvent*);

  virtual void mouseMoveEvent(QMouseEvent*);

  virtual void wheelEvent(QWheelEvent*);

  virtual void keyPressEvent(QKeyEvent*);

  virtual void focusInEvent(QFocusEvent*);

  virtual void focusOutEvent(QFocusEvent*); 

protected:
  virtual void Repaint(bool onlyOverlay = false );

  virtual void paintGL();

  virtual void initializeGL();

  virtual void resizeGL(int w, int h);

  virtual void showEvent(QShowEvent *);

  virtual void InitRenderer();

  virtual bool PrepareRendering();

  bool m_InitNeeded;

  bool m_ResizeNeeded;

  bool m_InResize;

  bool m_DrawOverlayOnly;

};

#endif /* QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66 */
