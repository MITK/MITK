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

#include "QmitkRenderingManager.h"


#include "mitkGeometry3D.h"
#include "mitkBaseRenderer.h"
#include "mitkSliceNavigationController.h"

#include <QApplication>
#include <QTimer>


QmitkRenderingManager
::QmitkRenderingManager()
{
  pendingTimerCallbacks=0;
}


void
QmitkRenderingManager
::DoMonitorRendering()
{
  
}


void
QmitkRenderingManager
::DoFinishAbortRendering()
{
  
}


QmitkRenderingManager
::~QmitkRenderingManager()
{
}


void
QmitkRenderingManager
::GenerateRenderingRequestEvent()
{
  QApplication::postEvent( this, new QmitkRenderingRequestEvent );
}


void
QmitkRenderingManager
::StartOrResetTimer()
{
  QTimer::singleShot(200, this, SLOT(TimerCallback()));
  pendingTimerCallbacks++;
}

void
QmitkRenderingManager
::TimerCallback()
{
  if(!--pendingTimerCallbacks)
    this->ExecutePendingHighResRenderingRequest();
}

bool 
QmitkRenderingManager
::event( QEvent *event ) 
{
  if ( event->type() == (QEvent::Type) QmitkRenderingRequestEvent::RenderingRequest )
{
    // Directly process all pending rendering requests
    this->ExecutePendingRequests();
    return true;
}

  return false;
}

