/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <qcursor.h>
#include <qlayout.h>
#include "mitkDisplayPositionEvent.h"
#include "mitkVtkLayerController.h"
#include "mitkRenderingManager.h"
#include "vtkRenderer.h"


QmitkRenderWindow::QmitkRenderWindow(QWidget *parent, const char *name, mitk::VtkPropRenderer* renderer)
: QVTKWidget(parent, name), 
  m_Renderer(renderer),
  m_ResendQtEvents(true)
{
  if(m_Renderer.IsNull())
  {
    std::string rendererName(name);
    m_Renderer = mitk::VtkPropRenderer::New( rendererName.c_str(), GetRenderWindow());
  }

  m_Renderer->InitRenderer(this->GetRenderWindow());

  mitk::BaseRenderer::AddInstance(GetRenderWindow(),m_Renderer);

  mitk::RenderingManager::GetInstance()->AddRenderWindow(GetRenderWindow());

  m_RenderProp = vtkMitkRenderProp::New();
  m_RenderProp->SetPropRenderer(m_Renderer);
  m_Renderer->GetVtkRenderer()->AddViewProp(m_RenderProp);
  
  if((this->GetRenderWindow()->GetSize()[0] > 10) 
      && (this->GetRenderWindow()->GetSize()[1] > 10))
    m_Renderer->InitSize(this->GetRenderWindow()->GetSize()[0], this->GetRenderWindow()->GetSize()[1]);

  
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);

  m_InResize = false;
}

QmitkRenderWindow::~QmitkRenderWindow()
{
  mitk::RenderingManager::GetInstance()->RemoveRenderWindow(GetRenderWindow());
  mitk::BaseRenderer::RemoveInstance(GetRenderWindow());
  m_Renderer->GetVtkRenderer()->RemoveViewProp(m_RenderProp);
  m_RenderProp->Delete();
}
  
mitk::VtkPropRenderer* QmitkRenderWindow::GetRenderer() 
{ 
  return m_Renderer; 
}
  
void QmitkRenderWindow::SetResendQtEvents(bool resend)
{
  m_ResendQtEvents = resend;
}

void QmitkRenderWindow::mousePressEvent(QMouseEvent *me) 
{
  QVTKWidget::mousePressEvent(me);
  if (m_Renderer.IsNotNull())
  {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MousePressEvent(&event);
  }
  
  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me) 
{
  QVTKWidget::mouseReleaseEvent(me);
  if (m_Renderer.IsNotNull()) 
  {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseReleaseEvent(&event);
  }
  
  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me) 
{
  QVTKWidget::mouseMoveEvent(me);
  if (m_Renderer.IsNotNull()) {
    mitk::Point2D p; p[0]=me->x(); p[1]=me->y();
    mitk::MouseEvent event(m_Renderer, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseMoveEvent(&event);
  }

  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  QVTKWidget::wheelEvent(we);
  
  if ( GetSliceNavigationController()->GetSliceLocked() )
    return;
  
  mitk::Stepper* stepper = GetSliceNavigationController()->GetSlice();
  
  if (stepper->GetSteps() <= 1)
  {
    stepper = GetSliceNavigationController()->GetTime();
  }

  if (we->orientation() * we->delta()  > 0) 
  {
    stepper->Next();
  }
  else
  {
    stepper->Previous();
  }
  
  if (m_ResendQtEvents) we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke) 
{
  QVTKWidget::keyPressEvent(ke);
  if (m_Renderer.IsNotNull())
  {
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(ke->type(), ke->key(), ke->state(), ke->text().isNull() ? "" : ke->text().ascii(), ke->isAutoRepeat(), ke->count(), cp.x(), cp.y(), QCursor::pos().x(), QCursor::pos().y());
    m_Renderer->KeyPressEvent(&mke);
    if(mke.isAccepted())
      ke->accept();
  }
  
  if (m_ResendQtEvents) ke->ignore();
}

void QmitkRenderWindow::InitRenderer()
{ 
}

void QmitkRenderWindow::resizeEvent(QResizeEvent* event)
{
  QVTKWidget::resizeEvent(event);

  if(m_InResize) //@FIXME CRITICAL probably related to VtkSizeBug
    return;
  m_InResize = true;

  if(this->isVisible())
  {
    if(m_Renderer.IsNotNull())
    {
      m_Renderer->Resize(event->size().width(), event->size().height());
    }

    //this->update();
    //updateGL();
  }

  m_InResize = false;
}


mitk::SliceNavigationController * QmitkRenderWindow::GetSliceNavigationController()
{
  return mitk::BaseRenderer::GetInstance(this->GetRenderWindow())->GetSliceNavigationController();
}
mitk::CameraRotationController * QmitkRenderWindow::GetCameraRotationController()
{
  return mitk::BaseRenderer::GetInstance(this->GetRenderWindow())->GetCameraRotationController();
}

mitk::BaseController * QmitkRenderWindow::GetController()
{
  mitk::BaseRenderer * renderer = mitk::BaseRenderer::GetInstance(GetRenderWindow());
  switch ( renderer->GetMapperID() )
  {
    case mitk::BaseRenderer::Standard2D:
      return GetSliceNavigationController();

    case mitk::BaseRenderer::Standard3D:
      return GetCameraRotationController();

    default:
      return GetSliceNavigationController();
  }
}

