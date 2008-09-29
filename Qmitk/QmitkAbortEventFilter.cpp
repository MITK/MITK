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
#include "mitkVolumeDataVtkMapper3D.h"
#include "QmitkAbortEventFilter.h"
#include "QmitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "QmitkRenderingManager.h"

#include "mitkNodePredicateProperty.h"
#include "mitkDataStorage.h"
#include "mitkProperties.h"

#include "QVTKWidget.h"

#include <qapplication.h>
#include <qeventloop.h>
#include <qguardedptr.h>

QmitkAbortEventFilter*
QmitkAbortEventFilter
::GetInstance()
{
  static QmitkAbortEventFilter instance;
  return &instance;
}

QmitkAbortEventFilter
::QmitkAbortEventFilter()
: m_LODRendererAtButtonPress( NULL )
{
}


QmitkAbortEventFilter
::~QmitkAbortEventFilter()
{
}


bool 
QmitkAbortEventFilter
::eventFilter( QObject *object, QEvent *event )
{   
  typedef QGuardedPtr< QObject > GuardedObject;

  // Extract renderer (if event has been invoked on a RenderWindow)
  bool isLODRenderer = false;
  mitk::BaseRenderer *renderer = NULL;
  QVTKWidget *qVTKWidget = dynamic_cast< QVTKWidget * >( object );
  if (qVTKWidget != NULL )
  {
    renderer = mitk::BaseRenderer::GetInstance( qVTKWidget->GetRenderWindow() );
    if ( renderer && renderer->GetNumberOfVisibleLODEnabledMappers() > 0 )
    {
      isLODRenderer = true;
    }
    else
    {
      // Only LOD enabled renderers are considered
      renderer = NULL;
    }

  }

  if (mitk::RenderingManager::GetInstance()->IsRendering() )
  {
    //m_EventQueue.push( ObjectEventPair(GuardedObject( object ), event) );
    //return true;

    switch ( event->type() )
    {
   
    case QEvent::MouseButtonPress:
    { 
      // Let only the first (RenderWindow specific) click event affect
      // the LOD process (Qt issues multiple events on mouse click, but the
      // RenderWindow specific one is issued first)
      if ( !m_ButtonPressed )
      {
        m_ButtonPressed = true;

        // Abort current rendering (for all rendering windows!)
        mitk::RenderingManager::GetInstance()->AbortRendering();

        // Block LOD increase until mouse button is released
        mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOn();

        // Store renderer (if LOD-enabled), otherwise renderer will be NULL
        m_LODRendererAtButtonPress = renderer;
      }

      // Store renderer if it is LOD enabled for mouse release
      if ( isLODRenderer )
      {
        m_LODRendererAtButtonPress = renderer;
      }

      QMouseEvent* me = ( QMouseEvent* )( event );
      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    
    case QEvent::MouseButtonDblClick:
    { 
      QMouseEvent* me = ( QMouseEvent* )( event );
      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::MouseMove:
    { 
      if ( m_ButtonPressed )
      {
        if ( isLODRenderer && mitk::RenderingManager::GetInstance()->GetNextLOD( m_LODRendererAtButtonPress ) != 0 )
        {
          mitk::RenderingManager::GetInstance()->AbortRendering();
        }
      }
      return true;
    }

    case QEvent::MouseButtonRelease:
    { 
      if ( m_ButtonPressed )
      {
        m_ButtonPressed = false;
        
        mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOff();
      }

      QMouseEvent* me = ( QMouseEvent* )( event );
      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::Wheel:
    {
      mitk::RenderingManager::GetInstance()->AbortRendering();

      QWheelEvent* we = ( QWheelEvent* )( event );
      QWheelEvent* newEvent = new QWheelEvent(
        we->pos(), we->globalPos(), we->delta(), we->state(), we->orientation()
      );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }    
   
    case QEvent::Paint:
    { 
      QPaintEvent* pe = ( QPaintEvent* )( event );
      QPaintEvent* newEvent = new QPaintEvent( pe->region() );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::KeyPress:
    { 
      mitk::RenderingManager::GetInstance()->AbortRendering();
      QKeyEvent* ke = ( QKeyEvent* )( event );
      QKeyEvent* newEvent = new QKeyEvent(
        ke->type(), ke->key(), ke->ascii(), ke->state(), ke->text(), false, ke->count()
      );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QmitkRenderingRequestEvent::RenderingRequest:
    {
      QmitkRenderingRequestEvent *newEvent = new QmitkRenderingRequestEvent();
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::ChildInserted: //change Layout (Big3D, 2D images up, etc.)	 
    {	 
      QChildEvent* ce = ( QChildEvent* )( event );	 
      QChildEvent* newEvent = new QChildEvent(	 
        QEvent::ChildInserted, ce->child() );	 
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;	 
    }	 

    case QEvent::ChildRemoved: //change Layout (Big3D, 2D images up, etc.)	 
    {	 
      QChildEvent* ce = ( QChildEvent* )( event );	 
      QChildEvent* newEvent = new QChildEvent(	 
        QEvent::ChildRemoved, ce->child() );	 
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;	 
    }	 

    case QEvent::Show:	 
    {	 
      QShowEvent* newEvent = new QShowEvent();	 
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;	 
    }	 

    case QEvent::Hide:	 
    {	 
      QHideEvent* newEvent = new QHideEvent();	 
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::Close:
    {
      QCloseEvent* newEvent = new QCloseEvent();
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::ContextMenu:
    {
      QContextMenuEvent *cme = ( QContextMenuEvent * )( event );
      QContextMenuEvent *newEvent = new QContextMenuEvent(
        cme->reason(), cme->pos(), cme->globalPos(), cme->state() );
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    case QEvent::Timer:
    { 
      QTimerEvent* te = ( QTimerEvent* )( event );
      QTimerEvent* newEvent = new QTimerEvent(te->timerId());
      m_EventQueue.push( ObjectEventPair(GuardedObject( object ), newEvent) );
      return true;
    }

    default:
    {
      return false;
    }
    }
  }
  else
  {
    switch ( event->type() )
    {
      case QEvent::MouseButtonPress:
      {
        // Let only the first (RenderWindow specific) click event affect
        // the LOD process (Qt issues multiple events on mouse click, but the
        // RenderWindow specific one is issued first)
        if ( !m_ButtonPressed )
        {
          m_ButtonPressed = true;
          mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOn();

          m_LODRendererAtButtonPress = renderer;
        }

        // Store renderer if it is LOD enabled for mouse release
        if ( isLODRenderer )
        {
          m_LODRendererAtButtonPress = renderer;
        }

        return false;
      }

      case QEvent::MouseMove:
      {
        // Nothing to do in this case
        return false;
      }

      case QEvent::Wheel:
      {
        //mitk::RenderingManager::GetInstance()->RequestUpdateAll(
        //  mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
        return false;
      }     

      case QEvent::MouseButtonRelease:
      {
        if ( m_ButtonPressed )
        {
          m_ButtonPressed = false;
          mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOff();

          if ( m_LODRendererAtButtonPress != NULL )
          {
            mitk::RenderingManager::GetInstance()->RequestUpdate(
              m_LODRendererAtButtonPress->GetRenderWindow() );
          }
          else
          {
            mitk::RenderingManager::GetInstance()->RequestUpdateAll(
              mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
          }
        }
        return false;
      }
    
      default:
      {
        return false;
      }
    }
  }
}


void 
QmitkAbortEventFilter
::ProcessEvents()
{ 
  qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
}


void 
QmitkAbortEventFilter
::IssueQueuedEvents()
{
  bool renderingRequested = false;
 
  while ( !m_EventQueue.empty() )
  {
    ObjectEventPair eventPair = m_EventQueue.front();
    if ( eventPair.first )
    {
      if ( eventPair.second->type() 
        == (QEvent::Type) QmitkRenderingRequestEvent::RenderingRequest )
      {
        // Rendering requests are issued only after all other events, to
        // ensure that they are executed BEFORE rendering, to avoid endless
        // loops
        renderingRequested = true;
      }
      else
      {
        QApplication::postEvent( eventPair.first, eventPair.second );
      }
    }
    m_EventQueue.pop();
  }

  // If a rendering request has been posed, issue it now at the end
  if ( renderingRequested )
  {
    QApplication::postEvent( this, new QmitkRenderingRequestEvent );
  }
}
