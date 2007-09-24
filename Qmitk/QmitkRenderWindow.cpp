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


#include "QmitkRenderWindow.h"
#include "mitkOpenGLRenderer.h"
#include "mitkRenderWindow.h"
#include <qcursor.h>
#include "mitkDisplayPositionEvent.h"
#include "QmitkRenderingManagerFactory.h"

QmitkRenderWindow::QmitkRenderWindow(mitk::BaseRenderer* renderer, QGLFormat glf, QWidget *parent, const char *name)
  : QGLWidget(glf, parent, name), mitk::RenderWindow(name, renderer), m_InitNeeded(false), m_ResizeNeeded(false), 
    m_InResize(false), m_DrawOverlayOnly(false)
{
  this->InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

QmitkRenderWindow::QmitkRenderWindow(QGLFormat glf, QWidget *parent, const char *name)
  : QGLWidget(glf, parent, name), mitk::RenderWindow(name, NULL), m_InitNeeded(false), m_ResizeNeeded(false), m_InResize(false), m_DrawOverlayOnly(false)

{
  this->InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

QmitkRenderWindow::QmitkRenderWindow(mitk::BaseRenderer* renderer, QWidget *parent, const char *name)
: QGLWidget(parent, name), mitk::RenderWindow(name, renderer), m_InitNeeded(false), m_ResizeNeeded(false), m_InResize(false), m_DrawOverlayOnly(false)

{
  this->InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent, const char *name)
: QGLWidget(parent, name), mitk::RenderWindow(name, NULL), m_InitNeeded(false), m_ResizeNeeded(false), m_InResize(false), m_DrawOverlayOnly(false)

{
  this->InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

QmitkRenderWindow::~QmitkRenderWindow()
{

}

void QmitkRenderWindow::InitRenderer()
{
  m_InitNeeded = true;
  m_ResizeNeeded = true;

  RenderWindow::InitRenderer();

  this->setAutoBufferSwap( false );
}
bool QmitkRenderWindow::PrepareRendering()
{
  // Get the native window ID and pass it
  // to the m_Renderer
  // before we render for the first time...
  if ( m_InitNeeded )
  {
    WId nId = winId();
    this->SetWindowId( (void*) nId );
    m_InitNeeded = false;
  }

  // rendering when not visible might cause serious problems (program may hang)
  if ( QGLWidget::isVisible() )
  {
    if ( m_Renderer.IsNotNull() )
    {
      return true; // allow rendering
    }
  }
  return false; // block rendering
}

/*!
\brief Initialize the OpenGL Window
*/
void QmitkRenderWindow::initializeGL() 
{
  if(m_Renderer.IsNotNull())
    m_Renderer->Initialize();
}

/*!
\brief Resize the OpenGL Window
*/
void QmitkRenderWindow::resizeGL( int w, int h ) 
{
  if(m_InResize) //@FIXME CRITICAL probably related to VtkSizeBug
    return;
  m_InResize = true;

  if(QGLWidget::isVisible())
  {
    if(m_Renderer.IsNotNull())
    {
      m_Renderer->Resize(w, h);
    }

    updateGL();
  }
  else
  {
    m_ResizeNeeded = true;
  }

  m_InResize = false;
}

/*!
\brief Render the scene
*/
void QmitkRenderWindow::paintGL( )
{
  // Calls back to PrepareRendering()
  m_Renderer->Render(m_DrawOverlayOnly);
}

void QmitkRenderWindow::showEvent ( QShowEvent * )
{
  // when the widget becomes visible the first time, we need to tell vtk the window size
  if(m_ResizeNeeded)
  {
    m_ResizeNeeded=false;
    if(m_Renderer.IsNotNull())
      m_Renderer->InitSize(width(),height());
  }
}


void QmitkRenderWindow::MakeCurrent()
{
  makeCurrent();
}

void QmitkRenderWindow::SwapBuffers() 
{
  swapBuffers();
};

bool QmitkRenderWindow::IsSharing () const
{
  return isSharing();
}

void QmitkRenderWindow::SetSize(int w, int h)
{
  if((w != width()) || (h != height()))
  {
    this->resize(w,h);
  }
  mitk::RenderWindow::SetSize(w,h);
}

void QmitkRenderWindow::mousePressEvent(QMouseEvent *me) 
{
  QGLWidget::mousePressEvent(me);
  if (m_Renderer.IsNotNull())
  {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MousePressEvent(&event);
  }
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me) 
{
  QGLWidget::mouseReleaseEvent(me);
  if (m_Renderer.IsNotNull()) 
  {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseReleaseEvent(&event);
  }
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me) 
{
  QGLWidget::mouseMoveEvent(me);
  if (m_Renderer.IsNotNull()) {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseMoveEvent(&event);
  }

  // Request additional rendering if left mouse button had been pressed.
  // This is required to make sure that the rendering is executed regularly;
  // since the rendering mechanism is generally controlled and executed via a 
  // timer, the timer might be blocked for longer periods if other Qt events
  // with higher priorities are in the pipeline.
  //
  // See also QmitkRenderingManager
  if ( me->state() & Qt::LeftButton )
  {
    mitk::RenderingManager::GetInstance()->UpdateCallback();
  }
}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  we->ignore();
  /*  QGLWidget::wheelEvent(we);
  if (m_Renderer.IsNotNull())
    m_Renderer->WheelEvent(we); */
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke) 
{
  if (m_Renderer.IsNotNull())
  {
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(ke->type(), ke->key(), ke->ascii(), ke->state(), ke->text().ascii(), ke->isAutoRepeat(), ke->count(), cp.x(), cp.y(), QCursor::pos().x(), QCursor::pos().y());
    m_Renderer->KeyPressEvent(&mke);
    if(mke.isAccepted())
      ke->accept();
  }
}

void QmitkRenderWindow::Repaint( bool onlyOverlay )
{
  m_DrawOverlayOnly = onlyOverlay;
  repaint();
}; 

QSize QmitkRenderWindow::minimumSizeHint () const
{
  return QSize(100, 100);
}

QSizePolicy QmitkRenderWindow::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, 0, 0);
}

QSize QmitkRenderWindow::sizeHint () const 
{
  return QSize(100, 100);
}

void QmitkRenderWindow::focusInEvent(QFocusEvent*)  {};
void QmitkRenderWindow::focusOutEvent(QFocusEvent*) {}; 

// Create and register RenderingManagerFactory for this platform.
QmitkRenderingManagerFactory qmitkRenderingManagerFactory;

//We have to put this in a file containing a class that is directly used
//somewhere. Otherwise, e.g. when put in VtkRenderWindowInteractor.cpp, 
//it is removed by the linker. 
#include "VtkQRenderWindowInteractor.h"
VtkQRenderWindowInteractorFactory vtkQRenderWindowInteractorFactory;

#include "QmitkApplicationCursor.h"
QmitkApplicationCursor globalQmitkApplicationCursor; // create one instance

#include "QmitkCallbackFromGUIThread.h"
QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;
