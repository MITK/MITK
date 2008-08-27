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

#include "mitkNodePredicateProperty.h"
#include "mitkDataStorage.h"
#include "mitkProperties.h"

#include <qapplication.h>
#include <qeventloop.h>

QmitkAbortEventFilter*
QmitkAbortEventFilter
::GetInstance()
{
  static QmitkAbortEventFilter instance;
  return &instance;
}

QmitkAbortEventFilter::QmitkAbortEventFilter()
{
}
QmitkAbortEventFilter::~QmitkAbortEventFilter()

{
}

bool QmitkAbortEventFilter::eventFilter( QObject *object, QEvent *event )
{   
  if (mitk::RenderingManager::GetInstance()->IsRendering() )
  {
    switch ( event->type() )
    {
   
    case QEvent::MouseButtonPress:
    { 
      m_ButtonPressed = true;

      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );

      mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
      mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOn();
      return true;
    }

    
    case QEvent::MouseButtonDblClick:
    { 
      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
      return true;
    }

    case QEvent::MouseButtonRelease:
    { 
      m_ButtonPressed = false;
      
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );

      mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOff();
      return true;
    }

    case QEvent::MouseMove:
    { 
      if ( m_ButtonPressed )
      {
        if ( mitk::RenderingManager::GetInstance()->GetCurrentLOD() != 0 )
        {
          mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
          mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
        }
      }
      return true;
    }

    case QEvent::Wheel:
    {
      mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QWheelEvent* we = ( QWheelEvent* )( event );

      QWheelEvent* newEvent = new QWheelEvent(
        we->pos(), we->globalPos(), we->delta(), we->state(), we->orientation()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
      return true;
    }    
   
    case QEvent::Resize:
      { 
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );

        QResizeEvent* re = ( QResizeEvent* )( event );

        QResizeEvent* newResizeEvent = new QResizeEvent( re->size(), re->oldSize() );
        m_EventQueue.push( ObjectEventPair(object, newResizeEvent) );
        return true;
      }

    case QEvent::Paint:
      { 
        QPaintEvent* pe = ( QPaintEvent* )( event );
        QPaintEvent* newPaintEvent = new QPaintEvent( pe->region() );
        m_EventQueue.push( ObjectEventPair(object, newPaintEvent) );
        return true;
      }

     case QEvent::ChildInserted: //change Layout (Big3D, 2D images up, etc.)
      {
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
        return false;
      }

      case QEvent::KeyPress:
      { 
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
        QKeyEvent* ke = ( QKeyEvent* )( event );
        QKeyEvent* newEvent = new QKeyEvent(
          ke->type(), ke->key(), ke->ascii(), ke->state(), ke->text(), false, ke->count()
        );
        m_EventQueue.push( ObjectEventPair(object, newEvent) );
        return true;
      }

      case QEvent::Timer:
      { 
        QTimerEvent* te = ( QTimerEvent* )( event );
        QTimerEvent* newEvent = new QTimerEvent(te->timerId());
        m_EventQueue.push( ObjectEventPair(object, newEvent) );
        return true;
      }

      default:
      {
        //std::cout<<"Event Type: (Rendering)"<<event->type()<<std::endl;
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
        m_ButtonPressed = true;
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
        mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOn();
        return false;
      }

      case QEvent::MouseMove:
      {
        if(m_ButtonPressed)
        {
          try
          {
            mitk::DataStorage* dataStorage = mitk::DataStorage::GetInstance();
            mitk::NodePredicateProperty VolRenTurnedOn("volumerendering", mitk::BoolProperty::New(true));
            mitk::DataStorage::SetOfObjects::ConstPointer VolRenSet = 
                dataStorage->GetSubset( VolRenTurnedOn );
            if ( VolRenSet->Size() > 0 )
            {
              mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
            }
          }
          catch(...){}
        }
        return false;
      }

      case QEvent::Wheel:
      {
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll(
          mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
        return false;
      }     

      case QEvent::MouseButtonRelease:
      {
        m_ButtonPressed = false;
        mitk::RenderingManager::GetInstance()->LODIncreaseBlockedOff();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll(
          mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
        return false;
      }
    
      case QEvent::Resize:
      {
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll(
          mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
        return false;
      } 

      case QEvent::ChildInserted:
      {
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
        return false;
      }

      default:
      {
        //std::cout<<"Event Type: (Not Rendering)"<<event->type()<<std::endl;
        return false;
      }
    }
  }
}

void QmitkAbortEventFilter::ProcessEvents()
{ 
  //std::cout << "P";
  qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
}

void QmitkAbortEventFilter::IssueQueuedEvents()
{
  while ( !m_EventQueue.empty() )
  {
    ObjectEventPair eventPair = m_EventQueue.front();
    QApplication::postEvent( eventPair.first, eventPair.second );
    m_EventQueue.pop();
  }

}
