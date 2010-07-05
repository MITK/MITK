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

#include <QCursor>
#include <QLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "QmitkEventAdapter.h"

#include "mitkDisplayPositionEvent.h"
#include "mitkVtkLayerController.h"
#include "mitkRenderingManager.h"
#include "vtkRenderer.h"

#include "QmitkRenderWindowMenu.h"

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent, QString name, mitk::VtkPropRenderer* renderer, mitk::RenderingManager* renderingManager )
: QVTKWidget(parent)
, m_Renderer(renderer)
, m_ResendQtEvents(true)
, m_MenuWidget(NULL)
, m_MenuWidgetActivated(false)
, m_ProcessWheelEvents(true)
{
  // if renderingManager is NULL get the global instance (needed here to initialize VtkPropRenderer below)
  if ( renderingManager == NULL )
  {
    renderingManager = mitk::RenderingManager::GetInstance();
  }

  if(m_Renderer.IsNull())
  {
      m_Renderer = mitk::VtkPropRenderer::New( qPrintable(name), GetRenderWindow(), renderingManager );
  }

  m_Renderer->InitRenderer(this->GetRenderWindow());

  mitk::BaseRenderer::AddInstance(GetRenderWindow(),m_Renderer);

  renderingManager->AddRenderWindow(GetRenderWindow());

  m_RenderProp = vtkMitkRenderProp::New();
  m_RenderProp->SetPropRenderer(m_Renderer);
  m_Renderer->GetVtkRenderer()->AddViewProp(m_RenderProp);

  if((this->GetRenderWindow()->GetSize()[0] > 10)
      && (this->GetRenderWindow()->GetSize()[1] > 10))
    m_Renderer->InitSize(this->GetRenderWindow()->GetSize()[0], this->GetRenderWindow()->GetSize()[1]);

  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  m_InResize = false;  

  //crate Renderwindow MenuBar for split, close Window or set new setting.
  m_MenuWidget = new QmitkRenderWindowMenu(this,0,m_Renderer);
  
  //create Signal/Slot Connection
  connect( m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)) );
  connect( m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()) );
  connect( m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)) );
}

QmitkRenderWindow::~QmitkRenderWindow()
{
  m_Renderer->GetRenderingManager()->RemoveRenderWindow(GetRenderWindow());
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

void QmitkRenderWindow::SetLayoutIndex( unsigned int layoutIndex )
{
  if( m_MenuWidget )
    m_MenuWidget->SetLayoutIndex(layoutIndex);
}

unsigned int QmitkRenderWindow::GetLayoutIndex()
{ 
  if( m_MenuWidget )
    return m_MenuWidget->GetLayoutIndex();
  else
    return NULL;
}

void QmitkRenderWindow::LayoutDesignListChanged( int layoutDesignIndex )
{ 
  if( m_MenuWidget ) 
    m_MenuWidget->UpdateLayoutDesignList( layoutDesignIndex );  
}

void QmitkRenderWindow::mousePressEvent(QMouseEvent *me)
{
  if (m_Renderer.IsNotNull())
  {
    mitk::MouseEvent event(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    m_Renderer->MousePressEvent(&event);
  }
  QVTKWidget::mousePressEvent(me);

  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me)
{
  if (m_Renderer.IsNotNull())
  {
    mitk::MouseEvent event(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    m_Renderer->MouseReleaseEvent(&event);
  }
  QVTKWidget::mouseReleaseEvent(me);

  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me)
{
  this->AdjustRenderWindowMenuVisibility( me->pos() );

  if (m_Renderer.IsNotNull()) {
    mitk::MouseEvent event(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    m_Renderer->MouseMoveEvent(&event);
  }
  QVTKWidget::mouseMoveEvent(me);

  //if (m_ResendQtEvents) me->ignore();
  ////Show/Hide Menu Widget
  //if( m_MenuWidgetActivated )
  //{
  //  //Show Menu Widget when mouse is inside of the define region of the top right corner
  //  if( m_MenuWidget->GetLayoutIndex() <= QmitkRenderWindowMenu::CORONAL 
  //    && me->pos().x() >= 0
  //    && me->pos().y() <= m_MenuWidget->height() + 20 )
  //  {
  //    m_MenuWidget->MoveWidgetToCorrectPos(1.0);
  //    m_MenuWidget->show();
  //    m_MenuWidget->update();
  //  }
  //  else if( m_MenuWidget->GetLayoutIndex() == QmitkRenderWindowMenu::THREE_D  
  //    && me->pos().x() >= this->width() - m_MenuWidget->width() - 20 
  //    && me->pos().y() <= m_MenuWidget->height() + 20 )
  //  {
  //    m_MenuWidget->MoveWidgetToCorrectPos(1.0);
  //    m_MenuWidget->show();
  //    m_MenuWidget->update();
  //  }
  //  //Hide Menu Widget when mouse is outside of the define region of the the right corner
  //  else if( !m_MenuWidget->GetSettingsMenuVisibilty() )
  //  {
  //    m_MenuWidget->hide();
  //  }    
  //}
}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  if ( !m_ProcessWheelEvents )
    return;
  //QVTKWidget::wheelEvent(we);

  if ( !GetSliceNavigationController()->GetSliceLocked() )
  {
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

    //also send to Renderer to send if to MITK interaction mechanism
    if (m_Renderer.IsNotNull()) 
    {
      mitk::WheelEvent event(QmitkEventAdapter::AdaptWheelEvent(m_Renderer, we));
      m_Renderer->WheelEvent(&event);
    }
  }

  QVTKWidget::wheelEvent(we);

  if (m_ResendQtEvents) 
    we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke)
{
  if (m_Renderer.IsNotNull())
  {
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(QmitkEventAdapter::AdaptKeyEvent(m_Renderer, ke, cp));
    m_Renderer->KeyPressEvent(&mke);
    ke->accept();
  }

  QVTKWidget::keyPressEvent(ke);

  if (m_ResendQtEvents) ke->ignore();
}

void QmitkRenderWindow::enterEvent( QEvent *e )
{
  QVTKWidget::enterEvent(e);
}

void QmitkRenderWindow::DeferredHideMenu( )
{
  MITK_DEBUG << "QmitkRenderWindow::DeferredHideMenu";

  if( m_MenuWidget )
    m_MenuWidget->HideMenu();
}

void QmitkRenderWindow::leaveEvent( QEvent *e )
{
  MITK_DEBUG << "QmitkRenderWindow::leaveEvent";

  //hide Menu Widget
  //if( m_MenuWidget->isVisible() && !m_MenuWidget->GetSettingsMenuVisibilty() && m_MenuWidgetActivated )
  //  m_MenuWidget->hide();
  
  QTimer::singleShot(0,this,SLOT( DeferredHideMenu( ) ) );

  QVTKWidget::leaveEvent(e);
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

  if(m_Renderer.IsNotNull())
  {
    m_Renderer->Resize(event->size().width(), event->size().height());
  }

  m_InResize = false;
}

void QmitkRenderWindow::AdjustRenderWindowMenuVisibility( const QPoint& pos )
{
  if ( !m_MenuWidgetActivated )
    return;

/*  if( m_MenuWidget->GetLayoutIndex() <= QmitkRenderWindowMenu::CORONAL 
    && pos.x() >= 0
    && pos.y() <= m_MenuWidget->height() + 20 )
  {
    m_MenuWidget->ShowMenu();
    m_MenuWidget->MoveWidgetToCorrectPos();
  }
  else if( m_MenuWidget->GetLayoutIndex() == QmitkRenderWindowMenu::THREE_D  
    && pos.x() >= this->width() - m_MenuWidget->width() - 20 
    && pos.y() <= m_MenuWidget->height() + 20 )
  {
    m_MenuWidget->MoveWidgetToCorrectPos();
    m_MenuWidget->ShowMenu();
  }
  //Hide Menu Widget when mouse is outside of the define region of the the right corner
  */
  
  int min = m_MenuWidget->height() + 20;
  int max = m_MenuWidget->height() + 64;

  int y=pos.y();
  
  if( y <= max )
  {
    float opacity = 1.0f-(y-min)/float(max-min);
    m_MenuWidget->ShowMenu();
    m_MenuWidget->MoveWidgetToCorrectPos(opacity);
  }
  else if( !m_MenuWidget->GetSettingsMenuVisibilty() )
  {
    m_MenuWidget->hide();
  }    
}

void QmitkRenderWindow::HideRenderWindowMenu( )
{
  if ( !m_MenuWidgetActivated )
    return;

  m_MenuWidget->hide();
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


void QmitkRenderWindow::OnChangeLayoutDesign( int layoutDesignIndex )
{
  
  emit SignalLayoutDesignChanged( layoutDesignIndex );
}


void QmitkRenderWindow::SetProcessWheelEvents( bool state )
{
  m_ProcessWheelEvents = state;
}

bool QmitkRenderWindow::GetProcessWheelEvents()
{
  return m_ProcessWheelEvents;
}


void QmitkRenderWindow::OnWidgetPlaneModeChanged( int mode )
{
  if( m_MenuWidget )
    m_MenuWidget->NotifyNewWidgetPlanesMode( mode );
}

void QmitkRenderWindow::FullScreenMode(bool state)
{
  if( m_MenuWidget )
    m_MenuWidget->ChangeFullScreenMode( state );
}  
