/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmlMitkRenderingManager.h"
#include "QmlMitkRenderWindowItem.h"


#include "mitkGeometry3D.h"
#include "mitkBaseRenderer.h"
#include "mitkSliceNavigationController.h"

#include <QCoreApplication>
#include <QTimer>


QmlMitkRenderingManager
::QmlMitkRenderingManager()
:pendingTimerCallbacks(0)
{
}

QmlMitkRenderingManager
::~QmlMitkRenderingManager()
{
}

void
QmlMitkRenderingManager
::GenerateRenderingRequestEvent()
{
  QCoreApplication::postEvent( this, new QmlMitkRenderingRequestEvent );
}


void
QmlMitkRenderingManager
::StartOrResetTimer()
{
  QTimer::singleShot(200, this, SLOT(TimerCallback()));
  pendingTimerCallbacks++;
}

void
QmlMitkRenderingManager
::TimerCallback()
{
  if(!--pendingTimerCallbacks)
  {
    this->ExecutePendingHighResRenderingRequest();
  }
}

#include <iostream>

bool
QmlMitkRenderingManager
::event( QEvent *event )
{
  if ( event->type() == (QEvent::Type) QmlMitkRenderingRequestEvent::RenderingRequest )
  {
    // Directly process all pending rendering requests
    //this->ExecutePendingRequests();
    this->MyUpdateExecutePendingRequests();

    return true;
  }

  return false;
}

#include "QVTKQuickItem.h"

void QmlMitkRenderingManager::MyUpdateExecutePendingRequests()
{
  m_UpdatePending = false;

  // Satisfy all pending update requests
  RenderWindowList::iterator it;
  int i = 0;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it, ++i )
  {
    if ( it->second == RENDERING_REQUESTED )
    {
      vtkRenderWindow* renderWindow = it->first;

        // If the renderWindow is not valid, we do not want to inadvertantly create
        // an entry in the m_RenderWindowList map. It is possible if the user is
        // regularly calling AddRenderer and RemoveRenderer for a rendering update
        // to come into this method with a renderWindow pointer that is valid in the
        // sense that the window does exist within the application, but that
        // renderWindow has been temporarily removed from this RenderingManager for
        // performance reasons.
        if (m_RenderWindowList.find( renderWindow ) == m_RenderWindowList.end())
        {
          continue;
        }

      // Erase potentially pending requests for this window
      m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;

      m_UpdatePending = false;

      // Immediately repaint this window (implementation platform specific)
      // If the size is 0 it crahses
      int *size = renderWindow->GetSize();
      if ( 0 != size[0] && 0 != size[1] )
      {
        QmlMitkRenderWindowItem* qqi = QmlMitkRenderWindowItem::GetInstanceForVTKRenderWindow( renderWindow );
        if (qqi)
        {
          qqi->update();
        }
      }
    }
  }
}


