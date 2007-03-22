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
  if ( mitk::RenderingManager::GetInstance()->IsRendering() )
  {
    switch ( event->type() )
    {
   
    case QEvent::MouseButtonPress:
    { std::cout<<"\nMouseButton\n";
      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
      return true;
    }
      return false;

      /*case QEvent::MouseMove:
    { std::cout<<"\nMove\n";
      mitk::RenderingManager::GetInstance()->AbortRendering( NULL );
      QMouseEvent* me = ( QMouseEvent* )( event );

      QMouseEvent* newEvent = new QMouseEvent(
        me->type(), me->pos(), me->globalPos(), me->button(), me->state()
      );
      m_EventQueue.push( ObjectEventPair(object, newEvent) );
      return true;
    }
      return false;*/
      
    case QEvent::Resize:
      { std::cout << "#R ";
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );

        QResizeEvent* re = ( QResizeEvent* )( event );

        QResizeEvent* newResizeEvent = new QResizeEvent( re->size(), re->oldSize() );
        m_EventQueue.push( ObjectEventPair(object, newResizeEvent) );
        return true;
      }
      return false;

    case QEvent::Paint:
      { std::cout << "#P ";
        mitk::RenderingManager::GetInstance()->AbortRendering( NULL );

        QPaintEvent* pe = ( QPaintEvent* )( event );

        QPaintEvent* newPaintEvent = new QPaintEvent( pe->region() );
        m_EventQueue.push( ObjectEventPair(object, newPaintEvent) );
      }
      return true;
    }
    return true;
  }
  return false;
}

void QmitkAbortEventFilter::ProcessEvents()
{ 
  std::cout << "P";
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
