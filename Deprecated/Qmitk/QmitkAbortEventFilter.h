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


#ifndef QMITKABORTEVENTFILTER_H_HEADER_INCLUDED_C135A197
#define QMITKABORTEVENTFILTER_H_HEADER_INCLUDED_C135A197

#include <qobject.h>
#include <utility>
#include <queue>
#include <iostream>
#include <mitkCommon.h>

namespace mitk
{
  class BaseRenderer;
}

class QMITK_EXPORT QmitkAbortEventFilter : public QObject
{
public:

  static QmitkAbortEventFilter* GetInstance();

  virtual ~QmitkAbortEventFilter();

  void ProcessEvents();
  void IssueQueuedEvents();
 
protected:
  QmitkAbortEventFilter();

  bool eventFilter( QObject* object, QEvent* event );

  bool m_ButtonPressed;
  
  bool m_MouseMoved;

  // Renderer of RenderWindow in which mouse button was pressed
  mitk::BaseRenderer *m_LODRendererAtButtonPress;
  
private:
  typedef std::pair< QObject*, QEvent* > ObjectEventPair;

  typedef std::queue< ObjectEventPair > EventQueue;

  EventQueue m_EventQueue;
};

class QMITK_EXPORT QmitkDebugEventFilter : public QObject
{
 protected:
   bool eventFilter( QObject* object, QEvent* event )
   {
     std::cout << "event:" << event->type() << ( event->spontaneous() ? "true" : "false" ) << " / " << object->className() << " / " << object->name() << std::endl; 
     return false;
   }
};

#endif /* QMITKABORTEVENTFILTER_H_HEADER_INCLUDED_C135A197 */
