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
{
  Initialize( renderingManager, name.toStdString().c_str() ); // Initialize mitkRenderWindowBase
 
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  //create render window MenuBar for split, close Window or set new setting.
  m_MenuWidget = new QmitkRenderWindowMenu(this,0,m_Renderer);

  //create Signal/Slot Connection
  connect( m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)) );
  connect( m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()) );
  connect( m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)) );
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
  
m_MenuWidget->smoothHide();  

  //hide Menu Widget
  //if( m_MenuWidget->isVisible() && !m_MenuWidget->GetSettingsMenuVisibilty() && m_MenuWidgetActivated )
  //  m_MenuWidget->hide();
  
  //QTimer::singleShot(10,this,SLOT( DeferredHideMenu( ) ) );



  QVTKWidget::leaveEvent(e);
}

void QmitkRenderWindow::resizeEvent(QResizeEvent* event)
{
  this->resizeMitkEvent(event->size().width(), event->size().height());
  
  QVTKWidget::resizeEvent(event);
}


void QmitkRenderWindow::moveEvent( QMoveEvent* event )
{
  QVTKWidget::moveEvent( event );

  // after a move the overlays need to be positioned
  emit Moved();
}


void QmitkRenderWindow::showEvent( QShowEvent* event )
{
  QVTKWidget::showEvent( event );

  // this singleshot is necessary to have the overlays positioned correctly after initial show
  // simple call of Moved() is no use here!!
  QTimer::singleShot(0, this ,SIGNAL( Moved() ) );
}


void QmitkRenderWindow::AdjustRenderWindowMenuVisibility( const QPoint& pos )
{

  
  MITK_DEBUG << "rw: adjustrender";
  
  
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
  
  /*int min = m_MenuWidget->height() + 20;
  int max = m_MenuWidget->height() + 64;

  int y=pos.y();
  
  float opacity;
  
  if( y <= max )
  {
    opacity = 1.0f-(y-min)/float(max-min);
  }
  else if( !m_MenuWidget->GetSettingsMenuVisibilty() )
  {
    opacity = 0;
  }    
  */
  m_MenuWidget->ShowMenu();
 // m_MenuWidget->MoveWidgetToCorrectPos(opacity);
  m_MenuWidget->MoveWidgetToCorrectPos(1.0f);
}

void QmitkRenderWindow::HideRenderWindowMenu( )
{
  MITK_DEBUG << "rw: hiderenderwindowmenu";
  
  if ( !m_MenuWidgetActivated )
    return;

 // m_MenuWidget->hide();
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
