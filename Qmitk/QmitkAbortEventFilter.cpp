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
#include "mitkVolumeDataVtkMapper3D.h"
#include "QmitkAbortEventFilter.h"
#include "QmitkRenderWindow.h"
#include "mitkRenderingManager.h"

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
      //std::cout << "#BP ";

      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
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
      //std::cout << "#BR ";
      
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
      return true;
    }

    case QEvent::MouseMove:
    { 
      if(m_ButtonPressed)
      {
        //std::cout << "#MM ";
        if(mitk::RenderingManager::GetInstance()->GetCurrentLOD()!=0)
        {
          mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
          mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
        }
      }
      return true;
    }
    
   
    case QEvent::Resize:
      { 
        //std::cout << "#R ";
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );

        QResizeEvent* re = ( QResizeEvent* )( event );

        QResizeEvent* newResizeEvent = new QResizeEvent( re->size(), re->oldSize() );
        m_EventQueue.push( ObjectEventPair(object, newResizeEvent) );
        return true;
      }

    case QEvent::Paint:
      { 
        //std::cout << "#P ";
        QPaintEvent* pe = ( QPaintEvent* )( event );
        QPaintEvent* newPaintEvent = new QPaintEvent( pe->region() );
        m_EventQueue.push( ObjectEventPair(object, newPaintEvent) );
        return true;
      }

     case QEvent::ShowWindowRequest:
      { 
        //std::cout << "#WR ";
        return false;
      }

      case QEvent::LayoutHint:
      { 
        //std::cout << "#L ";
        return false;
      }

      case QEvent::KeyPress:
      { 
        //std::cout << "#KP ";
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
        QKeyEvent* ke = ( QKeyEvent* )( event );
        QKeyEvent* newEvent = new QKeyEvent(
          ke->type(), ke->key(), ke->ascii(), ke->state(), ke->text(), false, ke->count()
        );
        m_EventQueue.push( ObjectEventPair(object, newEvent) );
        return true;
      }
    }

    return true;
  }
 else
  {
    if(event->type()==QEvent::MouseButtonPress)
    {
      m_ButtonPressed = true;
      //std::cout << "#BP2 ";
      return false;
    }

    if(event->type()==QEvent::MouseMove)
    {
      if(m_ButtonPressed){
        //std::cout << "#MM2 ";
        mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
      }
      return false;
    }

    if(event->type()==QEvent::MouseButtonRelease)
    {
      m_ButtonPressed = false;
      //std::cout << "#BR2 ";
      return false;
    }

    if(event->type()==QEvent::Resize)
    {
      //std::cout << "#R2 ";
      //mitk::RenderingManager::GetInstance()->SetCurrentLOD(0);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return false;
    } 

  }
 
  //std::cout<<"Event Type: (Not Rendering)"<<event->type()<<std::endl;
  
  return false;
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
