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
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "QmitkEventAdapter.h"

#include "QmitkRenderWindowMenu.h"

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent, QString name, mitk::VtkPropRenderer* /*renderer*/, mitk::RenderingManager* renderingManager )
: QVTKWidget(parent)
, m_ResendQtEvents(true)
, m_MenuWidget(NULL)
, m_MenuWidgetActivated(false)
, m_LayoutIndex(0)
{
  Initialize( renderingManager, name.toStdString().c_str() ); // Initialize mitkRenderWindowBase
 
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

 }

QmitkRenderWindow::~QmitkRenderWindow()
{
  Destroy(); // Destroy mitkRenderWindowBase
 
}

void QmitkRenderWindow::SetResendQtEvents(bool resend)
{
  m_ResendQtEvents = resend;
}

void QmitkRenderWindow::SetLayoutIndex( unsigned int layoutIndex )
{
  m_LayoutIndex = layoutIndex;
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
  mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
  this->mousePressMitkEvent(&myevent);
  
  QVTKWidget::mousePressEvent(me);

  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me)
{
  mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
  this->mouseReleaseMitkEvent(&myevent);
  
  QVTKWidget::mouseReleaseEvent(me);

  if (m_ResendQtEvents) me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me)
{
  this->AdjustRenderWindowMenuVisibility( me->pos() );

  
  mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
  this->mouseMoveMitkEvent(&myevent);
  
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
  mitk::WheelEvent myevent(QmitkEventAdapter::AdaptWheelEvent(m_Renderer, we));
  this->wheelMitkEvent(&myevent);

  QVTKWidget::wheelEvent(we);

  if (m_ResendQtEvents) 
    we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke)
{
  QPoint cp = mapFromGlobal(QCursor::pos());
  mitk::KeyEvent mke(QmitkEventAdapter::AdaptKeyEvent(m_Renderer, ke, cp));
  this->keyPressMitkEvent(&mke);
  ke->accept();

  QVTKWidget::keyPressEvent(ke);

  if (m_ResendQtEvents) ke->ignore();
}

void QmitkRenderWindow::enterEvent( QEvent *e )
{
MITK_DEBUG << "rw enter Event";

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
  
  if( m_MenuWidget )
    m_MenuWidget->smoothHide();  

  QVTKWidget::leaveEvent(e);
}

void QmitkRenderWindow::paintEvent(QPaintEvent* /*event*/)
{
  //We are using our own interaction and thus have to call the rendering manually.
  mitk::RenderingManager::GetInstance()->RequestUpdate(GetRenderWindow());
}

void QmitkRenderWindow::resizeEvent(QResizeEvent* event)
{
  this->resizeMitkEvent(event->size().width(), event->size().height());
  
  QVTKWidget::resizeEvent(event);

  emit resized();
}


void QmitkRenderWindow::moveEvent( QMoveEvent* event )
{
  QVTKWidget::moveEvent( event );

  // after a move the overlays need to be positioned
  emit moved();
}


void QmitkRenderWindow::showEvent( QShowEvent* event )
{
  QVTKWidget::showEvent( event );

  // this singleshot is necessary to have the overlays positioned correctly after initial show
  // simple call of moved() is no use here!!
  QTimer::singleShot(0, this ,SIGNAL( moved() ) );
}

void QmitkRenderWindow::ActivateMenuWidget( bool state )
{  
  m_MenuWidgetActivated = state; 
  
  if(!m_MenuWidgetActivated && m_MenuWidget)
  {
    //disconnect Signal/Slot Connection
    disconnect( m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)) );
    disconnect( m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()) );
    disconnect( m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)) );    
    
    delete m_MenuWidget;
    m_MenuWidget = 0;
  }
  else if(m_MenuWidgetActivated && !m_MenuWidget )
  {
    //create render window MenuBar for split, close Window or set new setting.
    m_MenuWidget = new QmitkRenderWindowMenu(this,0,m_Renderer);
    m_MenuWidget->SetLayoutIndex( m_LayoutIndex );
    
    //create Signal/Slot Connection
    connect( m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)) );
    connect( m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()) );
    connect( m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)) );    
  }
}

void QmitkRenderWindow::AdjustRenderWindowMenuVisibility( const QPoint& /*pos*/ )
{
  if( m_MenuWidget )
  {
    m_MenuWidget->ShowMenu();
    m_MenuWidget->MoveWidgetToCorrectPos(1.0f);
  }
}

void QmitkRenderWindow::HideRenderWindowMenu( )
{
  // DEPRECATED METHOD
}


void QmitkRenderWindow::OnChangeLayoutDesign( int layoutDesignIndex )
{
  emit SignalLayoutDesignChanged( layoutDesignIndex );
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
